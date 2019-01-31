/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDeviceManager.h
 * @author rdb
 * @date 2015-12-09
 */

#ifndef INPUTDEVICEMANAGER_H
#define INPUTDEVICEMANAGER_H

#include "pandabase.h"
#include "lightMutex.h"
#include "inputDevice.h"
#include "inputDeviceSet.h"

#ifdef _WIN32
#include "xInputDevice.h"
class WinRawInputDevice;
#endif

/**
 * This class keeps track of all the devices on a system, and sends out events
 * when a device has been hot-plugged.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_DEVICE InputDeviceManager {
protected:
  InputDeviceManager();
  ~InputDeviceManager() = default;

  static void make_global_ptr();

PUBLISHED:
  InputDeviceSet get_devices() const;
  InputDeviceSet get_devices(InputDevice::DeviceClass device_class) const;

  void add_device(InputDevice *device);
  void remove_device(InputDevice *device);

  virtual void update();

  INLINE static InputDeviceManager *get_global_ptr();

protected:
  LightMutex _lock;

#ifdef PHAVE_LINUX_INPUT_H
  InputDeviceSet _inactive_devices;
#endif

  InputDeviceSet _connected_devices;

  static InputDeviceManager *_global_ptr;
};

#include "inputDeviceManager.I"

#endif
