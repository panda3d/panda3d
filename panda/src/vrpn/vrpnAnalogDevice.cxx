// Filename: vrpnAnalogDevice.cxx
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

#include "vrpnAnalogDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnAnalogDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalogDevice::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnAnalogDevice::
VrpnAnalogDevice(VrpnClient *client, const string &device_name,
                 VrpnAnalog *vrpn_analog) :
  ClientAnalogDevice(client, device_name),
  _vrpn_analog(vrpn_analog)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalogDevice::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnAnalogDevice::
~VrpnAnalogDevice() {
  disconnect();
}
