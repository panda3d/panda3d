// Filename: vrpnTrackerDevice.cxx
// Created by:  drose (25Jan01)
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

#include "vrpnTrackerDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnTrackerDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VrpnTrackerDevice::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnTrackerDevice::
VrpnTrackerDevice(VrpnClient *client, const string &device_name,
                  int sensor, VrpnTrackerDevice::DataType data_type,
                  VrpnTracker *vrpn_tracker) :
  ClientTrackerDevice(client, device_name),
  _sensor(sensor),
  _data_type(data_type),
  _vrpn_tracker(vrpn_tracker)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTrackerDevice::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnTrackerDevice::
~VrpnTrackerDevice() {
  disconnect();
}
