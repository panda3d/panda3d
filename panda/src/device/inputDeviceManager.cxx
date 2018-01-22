/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDeviceManager.cxx
 * @author rdb
 * @date 2015-12-09
 */

#include "inputDeviceManager.h"
#include "ioKitInputDevice.h"
#include "linuxJoystickDevice.h"
#include "winInputDeviceManager.h"
#include "throw_event.h"

#ifdef PHAVE_LINUX_INPUT_H
#include <dirent.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#endif

InputDeviceManager *InputDeviceManager::_global_ptr = NULL;

/**
 * Initializes the input device manager by scanning which devices are currently
 * connected and setting up any platform-dependent structures necessary for
 * listening for future device connect events.
 */
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
#elif defined(__APPLE__)
InputDeviceManager::
InputDeviceManager() {
  _hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
  if (!_hid_manager) {
    device_cat.error()
      << "Failed to create an IOHIDManager.\n";
    return;
  }

  // The types of devices we're interested in.
  int page = kHIDPage_GenericDesktop;
  int usages[] = {kHIDUsage_GD_GamePad,
                  kHIDUsage_GD_Joystick,
                  kHIDUsage_GD_Mouse,
                  kHIDUsage_GD_Keyboard,
                  kHIDUsage_GD_MultiAxisController, 0};
  int *usage = usages;

  // This giant mess is necessary to create an array of match dictionaries
  // that will match the devices we're interested in.
  CFMutableArrayRef match = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
  nassertv(match);
  while (*usage) {
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFNumberRef page_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
    CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), page_ref);
    CFRelease(page_ref);
    CFNumberRef usage_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, usage);
    CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), usage_ref);
    CFRelease(usage_ref);
    CFArrayAppendValue(match, dict);
    CFRelease(dict);
    ++usage;
  }
  IOHIDManagerSetDeviceMatchingMultiple(_hid_manager, match);
  CFRelease(match);

  IOHIDManagerRegisterDeviceMatchingCallback(_hid_manager, on_match_device, this);
  IOHIDManagerScheduleWithRunLoop(_hid_manager, CFRunLoopGetMain(), kCFRunLoopCommonModes);
  IOHIDManagerOpen(_hid_manager, kIOHIDOptionsTypeNone);
}
#else
InputDeviceManager::
InputDeviceManager() : _lock("InputDeviceManager") {
}
#endif

/**
 * Closes any resources that the device manager was using to listen for events.
 */
InputDeviceManager::
~InputDeviceManager() {
#ifdef PHAVE_LINUX_INPUT_H
  if (_inotify_fd >= 0) {
    close(_inotify_fd);
    _inotify_fd = -1;
  }
#elif defined(__APPLE__)
  IOHIDManagerUnscheduleFromRunLoop(_hid_manager, CFRunLoopGetMain(), kCFRunLoopCommonModes);
  IOHIDManagerClose(_hid_manager, kIOHIDOptionsTypeNone);
  CFRelease(_hid_manager);
#endif
}

/**
 * Creates the global input manager.
 */
void InputDeviceManager::
make_global_ptr() {
#ifdef _WIN32
  _global_ptr = new WinInputDeviceManager;
#else
  _global_ptr = new InputDeviceManager;
#endif
}

#ifdef PHAVE_LINUX_INPUT_H
/**
 * Checks whether the event device with the given index is accessible, and if
 * so, adds it.  Returns the device if it was newly connected.
 *
 * This is the preferred interface for input devices on Linux.
 */
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

  // Nope.  The permissions haven't been configured to allow it.  Check if this
  // corresponds to a /dev/input/jsX interface, which has a better chance of
  // having read permissions set, but doesn't export all of the features
  // (notably, force feedback).

  // We do this by checking for a js# directory inside the sysfs directory.
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
          << "/dev/input/event" << ev_index << " is not readable, some "
             "features will be unavailable.\n";
      }
      _evdev_devices[ev_index] = device;
      return device;
    }
    entry = readdir(dir);
  }

  closedir(dir);
  return NULL;
}

/**
 * Checks whether the joystick device with the given index is accessible, and
 * if so, adds it.  Returns the device if it was newly connected.
 *
 * This is only used on Linux as a fallback interface for when an evdev device
 * cannot be accessed.
 */
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

/**
 * Description: Returns all currently connected devices.
 */
InputDeviceSet InputDeviceManager::
get_devices() const {
  InputDeviceSet devices;
  LightMutexHolder holder(_lock);

  for (size_t i = 0; i < _connected_devices.size(); ++i) {
    InputDevice *device = _connected_devices[i];
    devices.add_device(device);
  }

  return devices;
}

/**
 * Description: Returns all currently connected devices of the given device class.
 */
InputDeviceSet InputDeviceManager::
get_devices(InputDevice::DeviceClass device_class) const {
  InputDeviceSet devices;
  LightMutexHolder holder(_lock);

  for (size_t i = 0; i < _connected_devices.size(); ++i) {
    InputDevice *device = _connected_devices[i];
    if (device->get_device_class() == device_class) {
      devices.add_device(device);
    }
  }

  return devices;
}


/**
 * Called when a new device has been discovered.  This may also be used to
 * register virtual devices.
 *
 * This causes a connect-device event to be thrown.
 */
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

/**
 * Called when a device has been removed, or when a device should otherwise no
 * longer be tracked.
 *
 * This causes a disconnect-device event to be thrown.
 */
void InputDeviceManager::
remove_device(InputDevice *device) {
  {
    LightMutexHolder holder(_lock);
    _connected_devices.remove_device(device);
  }

  throw_event("disconnect-device", device);
}

/**
 * Polls the system to see if there are any new devices.  In some
 * implementations this is a no-op.
 */
void InputDeviceManager::
update() {
#ifdef PHAVE_LINUX_INPUT_H
  // Check for any devices that may be disconnected and need to be probed in
  // order to see whether they have been reconnected.
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
      // The device was created, or it was chmodded to be accessible.  We need
      // to check for the latter since it seems that the device may get the
      // IN_CREATE event before the driver gets the permissions set properly.

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
}

#if defined(__APPLE__) && !defined(CPPPARSER)
void InputDeviceManager::
on_match_device(void *ctx, IOReturn result, void *sender, IOHIDDeviceRef device) {
  InputDeviceManager *mgr = (InputDeviceManager *)ctx;
  nassertv(mgr != nullptr);
  nassertv(device);

  PT(InputDevice) input_device = new IOKitInputDevice(device);
  if (device_cat.is_debug()) {
    device_cat.debug()
      << "Discovered input device " << *input_device << "\n";
  }
  mgr->add_device(input_device);
}
#endif
