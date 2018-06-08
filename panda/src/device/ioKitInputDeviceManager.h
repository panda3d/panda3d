/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ioKitInputDeviceManager.h
 * @author rdb
 * @date 2018-02-04
 */

#ifndef IOKITINPUTDEVICEMANAGER_H
#define IOKITINPUTDEVICEMANAGER_H

#include "inputDeviceManager.h"

#if defined(__APPLE__) && !defined(CPPPARSER)
#include <IOKit/hid/IOHIDManager.h>

/**
 * The macOS implementation of InputDeviceManager.
 */
class EXPCL_PANDA_DEVICE IOKitInputDeviceManager final : public InputDeviceManager {
protected:
  IOKitInputDeviceManager();
  ~IOKitInputDeviceManager();

protected:
  IOHIDManagerRef _hid_manager;

  static void on_match_device(void *ctx, IOReturn result, void *sender, IOHIDDeviceRef device);

  friend class InputDeviceManager;
};

#endif

#endif
