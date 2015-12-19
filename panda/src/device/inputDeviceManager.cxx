// Filename: inputDeviceManager.cxx
// Created by:  rdb (09Dec15)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "inputDeviceManager.h"
#include "linuxJoystickDevice.h"
#include "throw_event.h"

#ifdef PHAVE_LINUX_INPUT_H
#include <dirent.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#endif

static ConfigVariableDouble xinput_detection_delay
("xinput-detection-delay", 0.5,
 PRC_DESC("How many seconds to wait between each check to see whether "
          "an XInput has been connected.  This check is not done every "
          "frame in order to prevent slowdown."));

InputDeviceManager *InputDeviceManager::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
#ifdef PHAVE_LINUX_INPUT_H
InputDeviceManager::
InputDeviceManager() : _inotify_fd(-1) {
  // Use inotify to watch /dev/input for hotplugging of devices.
  _inotify_fd = inotify_init1(O_NONBLOCK);

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
    while (entry != NULL) {
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
    _evdev_devices.resize(indices.back() + 1, NULL);

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
#elif defined(_WIN32)
InputDeviceManager::
InputDeviceManager() :
  _xinput_device0(0),
  _xinput_device1(1),
  _xinput_device2(2),
  _xinput_device3(3),
  _last_detection(0.0) {

  // These devices are bound to the lifetime of the input manager.
  _xinput_device0.local_object();
  _xinput_device1.local_object();
  _xinput_device2.local_object();
  _xinput_device3.local_object();

  if (_xinput_device0.is_connected()) {
    _connected_devices.add_device(&_xinput_device0);
  }
  if (_xinput_device1.is_connected()) {
    _connected_devices.add_device(&_xinput_device1);
  }
  if (_xinput_device2.is_connected()) {
    _connected_devices.add_device(&_xinput_device2);
  }
  if (_xinput_device3.is_connected()) {
    _connected_devices.add_device(&_xinput_device3);
  }
}
#else
InputDeviceManager::
InputDeviceManager() {
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::Destructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
InputDeviceManager::
~InputDeviceManager() {
#ifdef PHAVE_LINUX_INPUT_H
  if (_inotify_fd >= 0) {
    close(_inotify_fd);
    _inotify_fd = -1;
  }
#endif
}

#ifdef PHAVE_LINUX_INPUT_H
////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::consider_add_evdev_device
//       Access: Private
//  Description: Checks whether the event device with the given index
//               is accessible, and if so, adds it.  Returns the
//               device if it was newly connected.
////////////////////////////////////////////////////////////////////
InputDevice *InputDeviceManager::
consider_add_evdev_device(int ev_index) {
  if (ev_index < _evdev_devices.size()) {
    if (_evdev_devices[ev_index] != NULL) {
      // We already have this device.  FIXME: probe it and add it to the
      // list of connected devices?
      return NULL;
    }
  } else {
    // Make room to store this index.
    _evdev_devices.resize(ev_index + 1, NULL);
  }

  // Check if we can directly read the event device.
  char path[64];
  sprintf(path, "/dev/input/event%d", ev_index);

  if (access(path, R_OK) == 0) {
    PT(InputDevice) device = new EvdevInputDevice(ev_index);
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

  // Nope.  The permissions haven't been configured to allow it.
  // Check if this corresponds to a /dev/input/jsX interface, which has
  // a better chance of having read permissions set, but doesn't export
  // all of the features (notably, force feedback).

  // We do this by checking for a js# directory inside the sysfs
  // device directory.
  sprintf(path, "/sys/class/input/event%d/device", ev_index);

  DIR *dir = opendir(path);
  if (dir == NULL) {
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Error opening directory " << path << ": " << strerror(errno) << "\n";
    }
    return NULL;
  }

  dirent *entry = readdir(dir);
  while (entry != NULL) {
    int js_index = -1;
    if (sscanf(entry->d_name, "js%d", &js_index) == 1) {
      // Yes, we fonud a corresponding js device.  Try adding it.
      closedir(dir);

      InputDevice *device = consider_add_js_device(js_index);
      if (device != NULL && device_cat.is_warning()) {
        // This seemed to work.  Display a warning to the user indicating
        // that they might want to configure udev properly.
        device_cat.warning()
          << "Some features of " << device->get_device_class()
          << " device " << device->get_name() << " are not available due"
             " to lack of read permissions on /dev/input/event" << ev_index
          << ".\n";
      }
      _evdev_devices[ev_index] = device;
      return device;
    }
    entry = readdir(dir);
  }

  closedir(dir);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::consider_add_evdev_device
//       Access: Private
//  Description: Checks whether the joystick device with the given
//               index is accessible, and if so, adds it.  Returns
//               the device if it was newly connected.
////////////////////////////////////////////////////////////////////
InputDevice *InputDeviceManager::
consider_add_js_device(int js_index) {
  char path[64];
  sprintf(path, "/dev/input/js%d", js_index);

  if (access(path, R_OK) == 0) {
    PT(LinuxJoystickDevice) device = new LinuxJoystickDevice(js_index);
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

  return NULL;
}

#endif

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::get_gamepads
//       Access: Public
//  Description: Returns all currently connected gamepad devices.
////////////////////////////////////////////////////////////////////
InputDeviceSet InputDeviceManager::
get_gamepads() const {
  InputDeviceSet gamepads;
  LightMutexHolder holder(_lock);

  for (size_t i = 0; i < _connected_devices.size(); ++i) {
    InputDevice *device = _connected_devices[i];
    if (device->get_device_class() == InputDevice::DC_gamepad) {
      gamepads.add_device(device);
    }
  }

  return gamepads;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::add_device
//       Access: Published
//  Description: Called when a new device has been discovered.  This
//               may also be used to register virtual devices.
//
//               This sends a connect-device event.
////////////////////////////////////////////////////////////////////
void InputDeviceManager::
add_device(InputDevice *device) {
  {
    LightMutexHolder holder(_lock);
    _connected_devices.add_device(device);

#ifdef PHAVE_LINUX_INPUT_H
    // If we had added it pending activity on the device, remove it
    // from the list of inactive devices.
    _inactive_devices.remove_device(device);
#endif
  }
  throw_event("connect-device", device);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::remove_device
//       Access: Published
//  Description: Called when a device has been removed, or when a
//               device should otherwise no longer be tracked.
//
//               This sends a disconnect-device event.
////////////////////////////////////////////////////////////////////
void InputDeviceManager::
remove_device(InputDevice *device) {
  {
    LightMutexHolder holder(_lock);
    _connected_devices.remove_device(device);
  }

  throw_event("disconnect-device", device);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::update
//       Access: Public
//  Description: Polls the system to see if there are any new devices.
////////////////////////////////////////////////////////////////////
void InputDeviceManager::
update() {
#ifdef PHAVE_LINUX_INPUT_H
  // Check for any devices that may be disconnected and need to be probed
  // in order to see whether they have been reconnected.
  InputDeviceSet inactive_devices;
  {
    LightMutexHolder holder(_lock);
    inactive_devices = _inactive_devices;
  }
  for (size_t i = 0; i < inactive_devices.size(); ++i) {
    InputDevice *device = inactive_devices[i];
    if (device != NULL && !device->is_connected()) {
      device->poll();
    }
  }

  // We use inotify to tell us whether a device was added, removed,
  // or has changed permissions to allow us to access it.
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
          if (device != NULL) {
            _evdev_devices[index] = NULL;
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
      // The device was created, or it was chmodded to be accessible.  We
      // need to check for the latter since it seems that the device may
      // get the IN_CREATE event before the driver gets the permissions
      // set properly.

      int index = -1;
      if (sscanf(event->name, "event%d", &index) == 1) {
        InputDevice *device = consider_add_evdev_device(index);
        if (device != NULL && device->is_connected()) {
          throw_event("connect-device", device);
        }
      }
    }

    ptr += sizeof(inotify_event) + event->len;
  }
#endif

#ifdef _WIN32
  // XInput doesn't provide a very good hot-plugging interface.  We just
  // check if it's connected every so often.  Perhaps we can switch to
  // using RegisterDeviceNotification in the future.
  double time_now = ClockObject::get_global_clock()->get_real_time();
  LightMutexHolder holder(_update_lock);

  if (time_now - _last_detection > xinput_detection_delay.get_value()) {
    // I've heard this can be quite slow if no device is detected.  We
    // should probably move it to a thread.
    _xinput_device0.detect(this);
    _xinput_device1.detect(this);
    _xinput_device2.detect(this);
    _xinput_device3.detect(this);
    _last_detection = time_now;
  }
#endif
}
