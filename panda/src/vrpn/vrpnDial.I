// Filename: vrpnDial.I
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

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::get_dial_name
//       Access: Public
//  Description: Returns the name of the dial device that was used
//               to create this VrpnDial.
////////////////////////////////////////////////////////////////////
INLINE const string &VrpnDial::
get_dial_name() const {
  return _dial_name;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::is_empty
//       Access: Public
//  Description: Returns true if no VrpnDialDevices reference this
//               VrpnDial, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool VrpnDial::
is_empty() const {
  return _devices.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::poll
//       Access: Public
//  Description: Polls the connected device.  Normally you should not
//               call this directly; this will be called by the
//               VrpnClient.
////////////////////////////////////////////////////////////////////
INLINE void VrpnDial::
poll() {
  _dial->mainloop();
}
