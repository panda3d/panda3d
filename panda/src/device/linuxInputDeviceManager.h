/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linuxInputDeviceManager.h
 * @author rdb
 * @date 2018-01-25
 */

#ifndef LINUXINPUTDEVICEMANAGER_H
#define LINUXINPUTDEVICEMANAGER_H

#include "inputDeviceManager.h"

#ifdef PHAVE_LINUX_INPUT_H

/**
 * This class keeps track of all the devices on a system, and sends out events
 * when a device has been hot-plugged.
 */
class EXPCL_PANDA_DEVICE LinuxInputDeviceManager final : public InputDeviceManager {
private:
  LinuxInputDeviceManager();
  ~LinuxInputDeviceManager();

  InputDevice *consider_add_evdev_device(size_t index);
  InputDevice *consider_add_js_device(size_t index);

public:
  bool has_virtual_device(unsigned short vendor_id, unsigned short product_id) const;

  virtual void update();

protected:
  int _inotify_fd;

  pvector<InputDevice *> _evdev_devices;

  friend class InputDeviceManager;
};

#endif  // PHAVE_LINUX_INPUT_H

#endif
