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
    ((JointVertexTransform *)this)->_matrix = 
      _joint->_initial_net_transform_inverse *
      _joint->_net_transform;
    ((JointVertexTransform *)this)->_matrix_stale = false;
  }
}
