// Filename: vrpnTracker.I
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
