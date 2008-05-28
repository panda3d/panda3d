// Filename: vrpnTrackerDevice.I
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
//     Function: VrpnTrackerDevice::get_sensor
//       Access: Public
//  Description: Returns the particular sensor index that this device
//               wants to hear about from the VrpnTracker.
////////////////////////////////////////////////////////////////////
INLINE int VrpnTrackerDevice::
get_sensor() const {
  return _sensor;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTrackerDevice::get_data_type
//       Access: Public
//  Description: Returns the type of data this device represents from
//               the VrpnTracker.  This may be position, velocity, or
//               acceleration.
////////////////////////////////////////////////////////////////////
INLINE VrpnTrackerDevice::DataType VrpnTrackerDevice::
get_data_type() const {
  return _data_type;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTrackerDevice::get_vrpn_tracker
//       Access: Public
//  Description: Returns a pointer to the particular VrpnTracker this
//               device gets its data from.  This pointer may be
//               shared with other VrpnTrackerDevice objects (each
//               representing a different portion of the tracker
//               data).
////////////////////////////////////////////////////////////////////
INLINE VrpnTracker *VrpnTrackerDevice::
get_vrpn_tracker() const {
  return _vrpn_tracker;
}
