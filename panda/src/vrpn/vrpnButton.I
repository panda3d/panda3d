// Filename: vrpnButton.I
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
