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
#include "linuxInputDeviceManager.h"
#include "winInputDeviceManager.h"
#include "throw_event.h"

InputDeviceManager *InputDeviceManager::_global_ptr = NULL;

/**
 * Initializes the input device manager by scanning which devices are currently
 * connected and setting up any platform-dependent structures necessary for
 * listening for future device connect events.
 */
#if defined(__APPLE__)
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
#if defined(__APPLE__)
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
#elif defined(PHAVE_LINUX_INPUT_H)
  _global_ptr = new LinuxInputDeviceManager;
#else
  _global_ptr = new InputDeviceManager;
#endif
}

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
}
