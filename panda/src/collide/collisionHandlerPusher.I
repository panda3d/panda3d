// Filename: collisionHandlerPusher.I
// Created by:  drose (16Mar02)
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
