// Filename: clientTrackerDevice.I
// Created by:  drose (25Jan01)
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

