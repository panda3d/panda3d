// Filename: jointVertexTransform.I
// Created by:  drose (24Mar05)
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
//     Function: JointVertexTransform::get_joint
//       Access: Published
//  Description: Returns the joint for which this object returns the
//               transform.
////////////////////////////////////////////////////////////////////
INLINE const CharacterJoint *JointVertexTransform::
get_joint() const {
  return _joint;
}

////////////////////////////////////////////////////////////////////
//     Function: JointVertexTransform::check_matrix
//       Access: Private
//  Description: Recomputes _matrix if it needs it.
////////////////////////////////////////////////////////////////////
INLINE void JointVertexTransform::
check_matrix() const {
  if (_matrix_stale) {
    ((JointVertexTransform *)this)->compute_matrix();
  }
}
