/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnAnalogDevice.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "vrpnAnalogDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnAnalogDevice::_type_handle;

/**
 *
 */
VrpnAnalogDevice::
VrpnAnalogDevice(VrpnClient *client, const std::string &device_name,
                 VrpnAnalog *vrpn_analog) :
  ClientAnalogDevice(client, device_name),
  _vrpn_analog(vrpn_analog)
{
}

/**
 *
 */
VrpnAnalogDevice::
~VrpnAnalogDevice() {
  disconnect();
}
