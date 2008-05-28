// Filename: vrpnTracker.I
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
//     Function: VrpnTracker::get_tracker_name
//       Access: Public
//  Description: Returns the name of the tracker device that was used
//               to create this VrpnTracker.
////////////////////////////////////////////////////////////////////
INLINE const string &VrpnTracker::
get_tracker_name() const {
  return _tracker_name;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::is_empty
//       Access: Public
//  Description: Returns true if no VrpnTrackerDevices reference this
//               VrpnTracker, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool VrpnTracker::
is_empty() const {
  return _devices.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::poll
//       Access: Public
//  Description: Polls the connected device.  Normally you should not
//               call this directly; this will be called by the
//               VrpnClient.
////////////////////////////////////////////////////////////////////
INLINE void VrpnTracker::
poll() {
  _tracker->mainloop();
}
