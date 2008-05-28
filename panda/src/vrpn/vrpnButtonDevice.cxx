// Filename: vrpnButtonDevice.cxx
// Created by:  drose (26Jan01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "vrpnButtonDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnButtonDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VrpnButtonDevice::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnButtonDevice::
VrpnButtonDevice(VrpnClient *client, const string &device_name,
                 VrpnButton *vrpn_button) :
  ClientButtonDevice(client, device_name),
  _vrpn_button(vrpn_button)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnButtonDevice::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnButtonDevice::
~VrpnButtonDevice() {
  disconnect();
}
