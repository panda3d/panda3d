// Filename: vrpnButton.I
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
//     Function: VrpnButton::get_button_name
//       Access: Public
//  Description: Returns the name of the button device that was used
//               to create this VrpnButton.
////////////////////////////////////////////////////////////////////
INLINE const string &VrpnButton::
get_button_name() const {
  return _button_name;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnButton::is_empty
//       Access: Public
//  Description: Returns true if no VrpnButtonDevices reference this
//               VrpnButton, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool VrpnButton::
is_empty() const {
  return _devices.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnButton::poll
//       Access: Public
//  Description: Polls the connected device.  Normally you should not
//               call this directly; this will be called by the
//               VrpnClient.
////////////////////////////////////////////////////////////////////
INLINE void VrpnButton::
poll() {
  _button->mainloop();
}
