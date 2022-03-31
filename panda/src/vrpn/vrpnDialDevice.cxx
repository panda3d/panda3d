/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnDialDevice.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "vrpnDialDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnDialDevice::_type_handle;

/**
 *
 */
VrpnDialDevice::
VrpnDialDevice(VrpnClient *client, const std::string &device_name,
                 VrpnDial *vrpn_dial) :
  ClientDialDevice(client, device_name),
  _vrpn_dial(vrpn_dial)
{
}

/**
 *
 */
VrpnDialDevice::
~VrpnDialDevice() {
  disconnect();
}
