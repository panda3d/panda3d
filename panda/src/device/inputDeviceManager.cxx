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

InputDeviceManager *InputDeviceManager::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
#ifdef PHAVE_LINUX_INPUT_H
InputDeviceManager::
InputDeviceManager() : _inotify_fd(-1) {

  // Scan /dev/input for a list of input devices.
  DIR *dir = opendir("/dev/input");
  if (dir) {
    dirent *entry = readdir(dir);
    while (entry != NULL) {
      if (entry->d_type == DT_CHR) {
        string name(entry->d_name);
        if (!consider_add_linux_device(name)) {
          // We can't access it.  That's pretty normal for most devices.
          if (device_cat.is_debug()) {
            device_cat.debug()
              << "Ignoring input device /dev/input/" << name << ": "
              << strerror(errno) << "\n";
          }
          errno = 0;
        }
      }
      entry = readdir(dir);
    }
    closedir(dir);
  } else {
    device_cat.error()
      << "Error opening directory /dev/input: " << strerror(errno) << "\n";
    return;
  }

  // Use inotify to watch /dev/input for hotplugging of devices.
  _inotify_fd = inotify_init1(O_NONBLOCK);

  if (_inotify_fd < 0) {
    device_cat.error()
      << "Error initializing inotify: " << strerror(errno) << "\n";

  } else if (inotify_add_watch(_inotify_fd, "/dev/input", IN_CREATE | IN_ATTRIB | IN_DELETE) < 0) {
    device_cat.error()
      << "Error adding inotify watch on /dev/input: " << strerror(errno) << "\n";
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
//     Function: InputDeviceManager::consider_add_linux_device
//       Access: Private
//  Description: Checks whether the given device is accessible, and
//               if so, adds it.  Returns false on error.
////////////////////////////////////////////////////////////////////
bool InputDeviceManager::
consider_add_linux_device(const string &name) {
  // Get the full path name first.
  string path = "/dev/input/";
  path += name;

  if (access(path.c_str(), R_OK) < 0) {
    return false;
  }

  if (_devices_by_path.count(name)) {
    // We already have this device.
    return true;
  }

  // Check if it's a joystick or game controller device.
  if (name.size() > 2 && name[0] == 'j' && name[1] == 's' && isdigit(name[2])) {
    PT(InputDevice) device = new LinuxJoystickDevice(path);
    if (device_cat.is_info()) {
      device_cat.info()
        << "Discovered input device " << *device << "\n";
    }

    _devices_by_path[name] = device;
    _all_devices.push_back(MOVE(device));
    return true;
  }

  return true;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::add_device
//       Access: Public
//  Description: Called when a new device has been discovered.
////////////////////////////////////////////////////////////////////
void InputDeviceManager::
add_device(InputDevice *device) {
  {
    LightMutexHolder holder(_lock);
    _all_devices.push_back(device);
  }
  throw_event("device-added", device);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::remove_device
//       Access: Public
//  Description: Called when a device has been removed.
////////////////////////////////////////////////////////////////////
void InputDeviceManager::
remove_device(InputDevice *device) {
  {
    LightMutexHolder holder(_lock);

    InputDevices::iterator it;
    it = std::find(_all_devices.begin(), _all_devices.end(), device);
    nassertv_always(it != _all_devices.end());

    _all_devices.erase(it);
  }

  throw_event("device-removed", device);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceManager::poll
//       Access: Public
//  Description: Polls the system to see if there are any new devices.
////////////////////////////////////////////////////////////////////
void InputDeviceManager::
poll() {
#ifdef PHAVE_LINUX_INPUT_H
  // We use inotify to tell us whether a device was added, removed,
  // or has changed permissions to allow us to access it.
  unsigned int avail;
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
      DevicesByPath::iterator it = _devices_by_path.find(name);
      if (it != _devices_by_path.end()) {
        PT(InputDevice) device = it->second;
        device->set_connected(false);

        InputDevices::iterator it2;
        _devices_by_path.erase(it);
        it2 = std::find(_all_devices.begin(), _all_devices.end(), device);
        _all_devices.erase(it2);

        if (device_cat.is_info()) {
          device_cat.info()
            << "Removed input device " << *device << "\n";
        }
        throw_event("device-removed", device.p());
      }

    } else if (event->mask & (IN_CREATE | IN_ATTRIB)) {
      // The device was created, or it was chmodded to be accessible.
      DevicesByPath::iterator it = _devices_by_path.find(name);
      if (it == _devices_by_path.end()) {
        // We don't know about this device yet.
        if (!consider_add_linux_device(name) && (event->mask & IN_CREATE) != 0) {
          if (device_cat.is_debug()) {
            device_cat.debug()
              << "Ignoring input device /dev/input/" << name << ": "
              << strerror(errno) << "\n";
          }
        }
        errno = 0;
      }
    }

    ptr += sizeof(inotify_event) + event->len;
  }
#endif
}
