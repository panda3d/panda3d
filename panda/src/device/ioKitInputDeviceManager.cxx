/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ioKitInputDeviceManager.cxx
 * @author rdb
 * @date 2018-02-04
 */

#include "ioKitInputDeviceManager.h"
#include "ioKitInputDevice.h"

#if defined(__APPLE__) && !defined(CPPPARSER)

/**
 * Initializes the input device manager by scanning which devices are currently
 * connected and setting up any platform-dependent structures necessary for
 * listening for future device connect events.
 */
IOKitInputDeviceManager::
IOKitInputDeviceManager() {
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

/**
 * Closes any resources that the device manager was using to listen for events.
 */
IOKitInputDeviceManager::
~IOKitInputDeviceManager() {
  IOHIDManagerUnscheduleFromRunLoop(_hid_manager, CFRunLoopGetMain(), kCFRunLoopCommonModes);
  IOHIDManagerClose(_hid_manager, kIOHIDOptionsTypeNone);
  CFRelease(_hid_manager);
}

/**
 * Called by IOKit when an input device matching our filters has been found.
 */
void IOKitInputDeviceManager::
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
