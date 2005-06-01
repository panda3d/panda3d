// Filename: standardMunger.I
// Created by:  drose (21Mar05)
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
//     Function: StandardMunger::get_gsg
//       Access: Public
//  Description: Returns a pointer to the GSG that created this
//               munger.
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian *StandardMunger::
get_gsg() const {
  return _gsg;
}
