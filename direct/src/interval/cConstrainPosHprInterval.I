// Filename: cConstrainPosHprInterval.I
// Created by:  pratt (10Mar08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2008, Disney Enterprises, Inc.  All rights reserved
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
