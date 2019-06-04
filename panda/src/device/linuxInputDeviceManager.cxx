/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linuxInputDeviceManager.cxx
 * @author rdb
 * @date 2018-01-25
 */

#include "linuxInputDeviceManager.h"
#include "throw_event.h"

#ifdef PHAVE_LINUX_INPUT_H

#include "evdevInputDevice.h"
#include "linuxJoystickDevice.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>

/**
 * Initializes the device manager by scanning which devices are currently
 * connected and setting up any platform-dependent structures necessary for
 * listening for future device connect events.
 */
LinuxInputDeviceManager::
LinuxInputDeviceManager() {
  // Use inotify to watch /dev/input for hotplugging of devices.
  _inotify_fd = inotify_init();
  fcntl(_inotify_fd, F_SETFL, O_NONBLOCK);
  fcntl(_inotify_fd, F_SETFD, FD_CLOEXEC);

  if (_inotify_fd < 0) {
    device_cat.error()
      << "Error initializing inotify: " << strerror(errno) << "\n";

  } else if (inotify_add_watch(_inotify_fd, "/dev/input", IN_CREATE | IN_ATTRIB | IN_DELETE) < 0) {
    device_cat.error()
      << "Error adding inotify watch on /dev/input: " << strerror(errno) << "\n";
  }

  // Scan /dev/input for a list of input devices.
  DIR *dir = opendir("/dev/input");
  if (dir) {
    std::vector<size_t> indices;
    dirent *entry = readdir(dir);
    while (entry != nullptr) {
      size_t index;
      if (entry->d_type == DT_CHR && sscanf(entry->d_name, "event%zd", &index) == 1) {
        indices.push_back(index);
      }
      entry = readdir(dir);
    }
    closedir(dir);

    // We'll want to sort the devices by index, since the order may be
    // meaningful (eg. for the Xbox wireless receiver).
    if (indices.empty()) {
      return;
    }
    std::sort(indices.begin(), indices.end());
    _evdev_devices.resize(indices.back() + 1, nullptr);

    for (size_t index : indices) {
      consider_add_evdev_device(index);
    }
  } else {
    device_cat.error()
      << "Error opening directory /dev/input: " << strerror(errno) << "\n";
    return;
  }
}

/**
 * Closes any resources that the device manager was using to listen for events.
 */
LinuxInputDeviceManager::
~LinuxInputDeviceManager() {
  if (_inotify_fd >= 0) {
    close(_inotify_fd);
    _inotify_fd = -1;
  }
}

/**
 * Checks whether the event device with the given index is accessible, and if
 * so, adds it.  Returns the device if it was newly connected.
 *
 * This is the preferred interface for input devices on Linux.
 */
InputDevice *LinuxInputDeviceManager::
consider_add_evdev_device(size_t ev_index) {
  if (ev_index < _evdev_devices.size()) {
    if (_evdev_devices[ev_index] != nullptr) {
      // We already have this device.  FIXME: probe it and add it to the
      // list of connected devices?
      return nullptr;
    }
  } else {
    // Make room to store this index.
    _evdev_devices.resize(ev_index + 1, nullptr);
  }

  // Check if we can directly read the event device.
  char path[64];
  sprintf(path, "/dev/input/event%zd", ev_index);

  if (access(path, R_OK) == 0) {
    PT(InputDevice) device = new EvdevInputDevice(this, ev_index);
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Discovered evdev input device " << *device << "\n";
    }

    _evdev_devices[ev_index] = device;

    if (device->is_connected()) {
      _connected_devices.add_device(std::move(device));
    } else {
      // Wait for activity on the device before it is considered connected.
      _inactive_devices.add_device(std::move(device));
    }
    return _evdev_devices[ev_index];
  }

  // Nope.  The permissions haven't been configured to allow it.  Check if this
  // corresponds to a /dev/input/jsX interface, which has a better chance of
  // having read permissions set, but doesn't export all of the features
  // (notably, force feedback).

  // We do this by checking for a js# directory inside the sysfs directory.
  sprintf(path, "/sys/class/input/event%zd/device", ev_index);

  DIR *dir = opendir(path);
  if (dir == nullptr) {
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Error opening directory " << path << ": " << strerror(errno) << "\n";
    }
    return nullptr;
  }

  dirent *entry = readdir(dir);
  while (entry != nullptr) {
    size_t js_index;
    if (sscanf(entry->d_name, "js%zd", &js_index) == 1) {
      // Yes, we fonud a corresponding js device.  Try adding it.
      closedir(dir);

      InputDevice *device = consider_add_js_device(js_index);
      if (device != nullptr && device_cat.is_warning()) {
        // This seemed to work.  Display a warning to the user indicating
        // that they might want to configure udev properly.
        device_cat.warning()
          << "/dev/input/event" << ev_index << " is not readable, some "
             "features will be unavailable.\n";
      }
      _evdev_devices[ev_index] = device;
      return device;
    }
    entry = readdir(dir);
  }

  closedir(dir);
  return nullptr;
}

/**
 * Checks whether the joystick device with the given index is accessible, and
 * if so, adds it.  Returns the device if it was newly connected.
 *
 * This is only used on Linux as a fallback interface for when an evdev device
 * cannot be accessed.
 */
InputDevice *LinuxInputDeviceManager::
consider_add_js_device(size_t js_index) {
  char path[64];
  sprintf(path, "/dev/input/js%zd", js_index);

  if (access(path, R_OK) == 0) {
    PT(LinuxJoystickDevice) device = new LinuxJoystickDevice(this, js_index);
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Discovered joydev input device " << *device << "\n";
    }
    InputDevice *device_p = device.p();

    if (device->is_connected()) {
      _connected_devices.add_device(std::move(device));
    } else {
      // Wait for activity on the device before it is considered connected.
      _inactive_devices.add_device(std::move(device));
    }
    return device_p;
  }

  return nullptr;
}

/**
 * Scans the "virtual" input devices on the system to check whether one with
 * the given vendor and product ID exists.
 */
bool LinuxInputDeviceManager::
has_virtual_device(unsigned short vendor_id, unsigned short product_id) const {
  char path[294];
  sprintf(path, "/sys/devices/virtual/input");

  DIR *dir = opendir(path);
  if (dir != nullptr) {
    dirent *entry = readdir(dir);
    while (entry != nullptr) {
      if (entry->d_name[0] != 'i') {
        entry = readdir(dir);
        continue;
      }
      FILE *f;

      char vendor[5] = {0};
      sprintf(path, "/sys/devices/virtual/input/%s/id/vendor", entry->d_name);
      f = fopen(path, "r");
      if (f) {
        fgets(vendor, sizeof(vendor), f);
        fclose(f);
      }

      char product[5] = {0};
      sprintf(path, "/sys/devices/virtual/input/%s/id/product", entry->d_name);
      f = fopen(path, "r");
      if (f) {
        fgets(product, sizeof(product), f);
        fclose(f);
      }

      if (vendor[0] && std::stoi(std::string(vendor), nullptr, 16) == (int)vendor_id &&
          product[0] && std::stoi(std::string(product), nullptr, 16) == (int)product_id) {
        closedir(dir);
        return true;
      }

      entry = readdir(dir);
    }
    closedir(dir);
  }

  return false;
}

/**
 * Polls the system to see if there are any new devices.  In some
 * implementations this is a no-op.
 */
void LinuxInputDeviceManager::
update() {
  // Check for any devices that may be disconnected and need to be probed in
  // order to see whether they have been reconnected.
  InputDeviceSet inactive_devices;
  {
    LightMutexHolder holder(_lock);
    inactive_devices = _inactive_devices;
  }
  for (size_t i = 0; i < inactive_devices.size(); ++i) {
    InputDevice *device = inactive_devices[i];
    if (device != nullptr && !device->is_connected()) {
      device->poll();
    }
  }

  // We use inotify to tell us whether a device was added, removed, or has
  // changed permissions to allow us to access it.
  unsigned int avail = 0;
  ioctl(_inotify_fd, FIONREAD, &avail);
  if (avail == 0) {
    return;
  }

  char buffer[avail];
  int n_read = read(_inotify_fd, buffer, avail);
  if (n_read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available for now.

    } else {
      device_cat.error() << "read: " << strerror(errno) << "\n";
    }
    return;
  }

  LightMutexHolder holder(_lock);

  // Iterate over the events in the buffer.
  bool removed_steam_virtual_device = false;
  char *ptr = buffer;
  char *end = buffer + avail;
  while (ptr < end) {
    inotify_event *event = (inotify_event *)ptr;

    std::string name(event->name);

    if (event->mask & IN_DELETE) {
      // The device was deleted.  If we have it, remove it.

      size_t index;
      if (sscanf(event->name, "event%zd", &index) == 1) {
        // Check if we have this evdev device.  If so, disconnect it.
        if (index < _evdev_devices.size()) {
          PT(InputDevice) device = _evdev_devices[index];
          if (device != nullptr) {
            device->set_connected(false);
            _evdev_devices[index] = nullptr;
            _inactive_devices.remove_device(device);
            if (_connected_devices.remove_device(device)) {
              throw_event("disconnect-device", device.p());
            }

            if (device_cat.is_debug()) {
              device_cat.debug()
                << "Removed input device " << *device << "\n";
            }

            // Check for Steam virtual device; see comment below.
            if (device->get_vendor_id() == 0x28de &&
                device->get_product_id() == 0x11ff) {
              removed_steam_virtual_device = true;
            }
          }
        }
      }

    } else if (event->mask & (IN_CREATE | IN_ATTRIB)) {
      // The device was created, or it was chmodded to be accessible.  We need
      // to check for the latter since it seems that the device may get the
      // IN_CREATE event before the driver gets the permissions set properly.

      size_t index;
      if (sscanf(event->name, "event%zd", &index) == 1) {
        InputDevice *device = consider_add_evdev_device(index);
        if (device != nullptr && device->is_connected()) {
          throw_event("connect-device", device);
        }
      }
    }

    ptr += sizeof(inotify_event) + event->len;
  }

  // If the Steam virtual device was just disconnected, the user may have just
  // shut down Steam, and we need to reactivate the real Steam Controller
  // device that was previously suppressed by Steam.
  if (removed_steam_virtual_device) {
    inactive_devices = _inactive_devices;

    for (size_t i = 0; i < inactive_devices.size(); ++i) {
      InputDevice *device = inactive_devices[i];
      if (device != nullptr && device->is_of_type(EvdevInputDevice::get_class_type())) {
        PT(EvdevInputDevice) evdev_device = (EvdevInputDevice *)device;
        if (evdev_device->reactivate_steam_controller()) {
          _inactive_devices.remove_device(device);
          _connected_devices.add_device(device);
          throw_event("connect-device", device);
        }
      }
    }
  }
}

#endif  // PHAVE_LINUX_INPUT_H
