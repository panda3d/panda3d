// Filename: decalTransition.I
// Created by:  drose (17Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: DecalTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DecalTransition::
DecalTransition() {
  set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: DecalTransition::off
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DecalTransition DecalTransition::
off() {
  DecalTransition result;
  result.set_off();
  return result;
}
