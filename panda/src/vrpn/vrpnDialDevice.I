// Filename: vrpnDialDevice.I
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
//     Function: VrpnDialDevice::get_vrpn_dial
//       Access: Public
//  Description: Returns a pointer to the particular VrpnDial this
//               device gets its data from.  This pointer may be
//               shared with other VrpnDialDevice objects.
////////////////////////////////////////////////////////////////////
INLINE VrpnDial *VrpnDialDevice::
get_vrpn_dial() const {
  return _vrpn_dial;
}
