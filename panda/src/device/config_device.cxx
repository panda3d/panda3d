/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_device.cxx
 * @author drose
 * @date 2000-05-04
 */

#include "config_device.h"
#include "analogNode.h"
#include "buttonNode.h"
#include "clientAnalogDevice.h"
#include "clientBase.h"
#include "clientButtonDevice.h"
#include "clientDevice.h"
#include "clientDialDevice.h"
#include "clientTrackerDevice.h"
#include "dialNode.h"
#include "evdevInputDevice.h"
#include "inputDevice.h"
#include "linuxJoystickDevice.h"
#include "trackerNode.h"
#include "virtualMouse.h"
#include "xInputDevice.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_DEVICE)
  #error Buildsystem error: BUILDING_PANDA_DEVICE not defined
#endif

Configure(config_device);
NotifyCategoryDef(device, "");

ConfigVariableBool asynchronous_clients
("asynchronous-clients", true);

ConfigureFn(config_device) {
  init_libdevice();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libdevice() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  AnalogNode::init_type();
  ButtonNode::init_type();
  ClientAnalogDevice::init_type();
  ClientBase::init_type();
  ClientButtonDevice::init_type();
  ClientDevice::init_type();
  ClientDialDevice::init_type();
  ClientTrackerDevice::init_type();
  DialNode::init_type();
  InputDevice::init_type();
  TrackerNode::init_type();
  VirtualMouse::init_type();

#ifdef PHAVE_LINUX_INPUT_H
  EvdevInputDevice::init_type();
  LinuxJoystickDevice::init_type();
#endif

#ifdef _WIN32
  XInputDevice::init_type();
#endif
}
