// Filename: vrpnButtonDevice.cxx
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
