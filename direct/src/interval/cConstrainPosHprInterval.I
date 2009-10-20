// Filename: cConstrainPosHprInterval.I
// Created by:  pratt (10Mar08)
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
//     Function: CConstrainPosHprInterval::get_node
//       Access: Published
//  Description: Returns the "source" node.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &CConstrainPosHprInterval::
get_node() const {
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: CConstrainPosHprInterval::get_target
//       Access: Published
//  Description: Returns the "target" node.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &CConstrainPosHprInterval::
get_target() const {
  return _target;
}
