// Filename: vrpnAnalogDevice.cxx
// Created by:  drose (26Jan01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
