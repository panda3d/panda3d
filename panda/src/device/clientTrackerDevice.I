// Filename: clientTrackerDevice.I
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


////////////////////////////////////////////////////////////////////
//     Function: ClientTrackerDevice::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ClientTrackerDevice::
ClientTrackerDevice(ClientBase *client, const string &device_name):
  ClientDevice(client, get_class_type(), device_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ClientTrackerDevice::get_data
//       Access: Public
//  Description: Returns the TrackerData that this device is
//               reporting.
////////////////////////////////////////////////////////////////////
INLINE const TrackerData &ClientTrackerDevice::
get_data() const {
  return _data;
}

