/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnButtonDevice.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "vrpnButtonDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnButtonDevice::_type_handle;

/**
 *
 */
VrpnButtonDevice::
VrpnButtonDevice(VrpnClient *client, const std::string &device_name,
                 VrpnButton *vrpn_button) :
  ClientButtonDevice(client, device_name),
  _vrpn_button(vrpn_button)
{
}

/**
 *
 */
VrpnButtonDevice::
~VrpnButtonDevice() {
  disconnect();
}
