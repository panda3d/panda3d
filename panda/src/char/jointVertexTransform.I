// Filename: jointVertexTransform.I
// Created by:  drose (24Mar05)
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
//     Function: JointVertexTransform::get_from
//       Access: Published
//  Description: Returns the joint whose coordinate space this object
//               moves vertices from.
////////////////////////////////////////////////////////////////////
INLINE const CharacterJoint *JointVertexTransform::
get_from() const {
  return _from;
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::get_to
//       Access: Published
//  Description: Returns the joint whose coordinate space this object
//               moves vertices into.
////////////////////////////////////////////////////////////////////
INLINE const CharacterJoint *JointVertexTransform::
get_to() const {
  return _to;
}
