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

static ConfigVariableBool iokit_scan_mouse_devices
("iokit-scan-mouse-devices", false,
 PRC_DESC("Set this to true to enable capturing raw mouse data via IOKit on "
          "macOS.  This is disabled by default because newer macOS versions "
          "will prompt the user explicitly for permissions when this is on."));

static ConfigVariableBool iokit_scan_keyboard_devices
("iokit-scan-keyboard-devices", false,
 PRC_DESC("Set this to true to enable capturing raw keyboard data via IOKit on "
          "macOS.  This is disabled by default because newer macOS versions "
          "will prompt the user explicitly for permissions when this is on."));

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
                  kHIDUsage_GD_MultiAxisController,
                  0, 0, 0};

  int num_usages = 3;
  if (iokit_scan_mouse_devices) {
    usages[num_usages++] = kHIDUsage_GD_Mouse;
  }
  if (iokit_scan_keyboard_devices) {
    usages[num_usages++] = kHIDUsage_GD_Keyboard;
  }

  // This giant mess is necessary to create an array of match dictionaries
  // that will match the devices we're interested in.
  CFMutableArrayRef match = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
  nassertv(match);
  int *usage = usages;
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
  IOHIDManagerRegisterDeviceRemovalCallback(_hid_manager, on_remove_device, this);
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
  IOKitInputDeviceManager *mgr = (IOKitInputDeviceManager *)ctx;
  nassertv(mgr != nullptr);
  nassertv(device);

  IOKitInputDevice *input_device = new IOKitInputDevice(device);
  if (device_cat.is_debug()) {
    device_cat.debug()
      << "Discovered input device " << *input_device << "\n";
  }
  mgr->add_device(input_device);
  mgr->_devices_by_hidref[device] = input_device;
}

/**
 * Called by IOKit when an input device has disappeared.
 */
void IOKitInputDeviceManager::
on_remove_device(void *ctx, IOReturn result, void *sender, IOHIDDeviceRef device) {
  IOKitInputDeviceManager *mgr = (IOKitInputDeviceManager *)ctx;
  nassertv(mgr != nullptr);
  nassertv(device);

  auto it = mgr->_devices_by_hidref.find(device);
  nassertv(it != mgr->_devices_by_hidref.end());
  IOKitInputDevice *input_device = it->second;

  input_device->set_connected(false);

  mgr->_devices_by_hidref.erase(device);

  IOHIDDeviceClose(device, kIOHIDOptionsTypeNone);

  if (device_cat.is_debug()) {
    device_cat.debug()
      << "Removed input device " << *input_device << "\n";
  }

  mgr->remove_device(input_device);
}
#endif
