// Filename: collisionHandlerPusher.I
// Created by:  drose (16Mar02)
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
//     Function: CollisionHandlerPusher::set_horizontal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPusher::
set_horizontal(bool flag) {
  _horizontal = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPusher::get_horizontal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CollisionHandlerPusher::
get_horizontal() const {
  return _horizontal;
}
