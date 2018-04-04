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

#include "inputDeviceManager.h"
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
  fcntl(_inotify_fd, O_NONBLOCK | O_CLOEXEC);

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
    vector_int indices;
    dirent *entry = readdir(dir);
    while (entry != nullptr) {
      int index = -1;
      if (entry->d_type == DT_CHR && sscanf(entry->d_name, "event%d", &index) == 1) {
        indices.push_back(index);
      }
      entry = readdir(dir);
    }
    closedir(dir);

    // We'll want to sort the devices by index, since the order may be
    // meaningful (eg. for the Xbox wireless receiver).
    std::sort(indices.begin(), indices.end());
    _evdev_devices.resize(indices.back() + 1, nullptr);

    vector_int::const_iterator it;
    for (it = indices.begin(); it != indices.end(); ++it) {
      consider_add_evdev_device(*it);
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
consider_add_evdev_device(int ev_index) {
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
  sprintf(path, "/dev/input/event%d", ev_index);

  if (access(path, R_OK) == 0) {
    PT(InputDevice) device = new EvdevInputDevice(this, ev_index);
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Discovered evdev input device " << *device << "\n";
    }

    _evdev_devices[ev_index] = device;

    if (device->is_connected()) {
      _connected_devices.add_device(MOVE(device));
    } else {
      // Wait for activity on the device before it is considered connected.
      _inactive_devices.add_device(MOVE(device));
    }
    return _evdev_devices[ev_index];
  }

  // Nope.  The permissions haven't been configured to allow it.  Check if this
  // corresponds to a /dev/input/jsX interface, which has a better chance of
  // having read permissions set, but doesn't export all of the features
  // (notably, force feedback).

  // We do this by checking for a js# directory inside the sysfs directory.
  sprintf(path, "/sys/class/input/event%d/device", ev_index);

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
    int js_index = -1;
    if (sscanf(entry->d_name, "js%d", &js_index) == 1) {
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
consider_add_js_device(int js_index) {
  char path[64];
  sprintf(path, "/dev/input/js%d", js_index);

  if (access(path, R_OK) == 0) {
    PT(LinuxJoystickDevice) device = new LinuxJoystickDevice(this, js_index);
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Discovered joydev input device " << *device << "\n";
    }
    InputDevice *device_p = device.p();

    if (device->is_connected()) {
      _connected_devices.add_device(MOVE(device));
    } else {
      // Wait for activity on the device before it is considered connected.
      _inactive_devices.add_device(MOVE(device));
    }
    return device_p;
  }

  return nullptr;
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
  char *ptr = buffer;
  char *end = buffer + avail;
  while (ptr < end) {
    inotify_event *event = (inotify_event *)ptr;

    string name(event->name);

    if (event->mask & IN_DELETE) {
      // The device was deleted.  If we have it, remove it.

      int index = -1;
      if (sscanf(event->name, "event%d", &index) == 1) {
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
          }
        }
      }

    } else if (event->mask & (IN_CREATE | IN_ATTRIB)) {
      // The device was created, or it was chmodded to be accessible.  We need
      // to check for the latter since it seems that the device may get the
      // IN_CREATE event before the driver gets the permissions set properly.

      int index = -1;
      if (sscanf(event->name, "event%d", &index) == 1) {
        InputDevice *device = consider_add_evdev_device(index);
        if (device != nullptr && device->is_connected()) {
          throw_event("connect-device", device);
        }
      }
    }

    ptr += sizeof(inotify_event) + event->len;
  }
}

#endif  // PHAVE_LINUX_INPUT_H
