/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ioKitInputDevice.h
 * @author rdb
 * @date 2017-12-21
 */

#ifndef IOKITINPUTDEVICE_H
#define IOKITINPUTDEVICE_H

#include "pandabase.h"
#include "inputDevice.h"

#if defined(__APPLE__) && !defined(CPPPARSER)

#include <IOKit/hid/IOHIDDevice.h>

/**
 * This implementation uses the IOKit HID code that was introduced with macOS
 * 10.5 to interface with USB HID devices.
 */
class EXPCL_PANDA_DEVICE IOKitInputDevice final : public InputDevice {
public:
  IOKitInputDevice(IOHIDDeviceRef device);
  ~IOKitInputDevice();

  void on_remove();

private:
  void parse_element(IOHIDElementRef element);

  virtual void do_poll();

  IOHIDDeviceRef _device;
  pvector<IOHIDElementRef> _button_elements;
  pvector<IOHIDElementRef> _analog_elements;
  IOHIDElementRef _hat_element;
  int _hat_left_button;
  IOHIDElementRef _pointer_x;
  IOHIDElementRef _pointer_y;
  IOHIDElementRef _scroll_wheel;
  uint64_t _pointer_x_timestamp;
  uint64_t _pointer_y_timestamp;
  uint64_t _scroll_wheel_timestamp;
};

#endif  // __APPLE__

#endif
