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
#include "ioKitInputDeviceManager.h"
#include "linuxInputDeviceManager.h"
#include "winInputDeviceManager.h"
#include "throw_event.h"
#include "config_putil.h"

InputDeviceManager *InputDeviceManager::_global_ptr = nullptr;

/**
 * Initializes the input device manager by scanning which devices are currently
 * connected and setting up any platform-dependent structures necessary for
 * listening for future device connect events.
 */
InputDeviceManager::
InputDeviceManager() : _lock("InputDeviceManager") {
}

/**
 * Creates the global input manager.
 */
void InputDeviceManager::
make_global_ptr() {
  init_libputil();

#ifdef _WIN32
  _global_ptr = new WinInputDeviceManager;
#elif defined(__APPLE__)
  _global_ptr = new IOKitInputDeviceManager;
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
