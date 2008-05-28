// Filename: vrpnDialDevice.cxx
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

#include "vrpnDialDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnDialDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VrpnDialDevice::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnDialDevice::
VrpnDialDevice(VrpnClient *client, const string &device_name,
                 VrpnDial *vrpn_dial) :
  ClientDialDevice(client, device_name),
  _vrpn_dial(vrpn_dial)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDialDevice::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnDialDevice::
~VrpnDialDevice() {
  disconnect();
}
