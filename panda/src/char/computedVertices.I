// Filename: computedVertices.I
// Created by:  drose (01Mar99)
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
//     Function: ComputedVertices::VertexTransform::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVertices::VertexTransform::
VertexTransform() {
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVertices::
ComputedVertices() {
}

////////////////////////////////////////////////////////////////////
//     Function: ComputedVertices::VertexTransform::Ordering operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ComputedVertices::VertexTransform::
operator < (const ComputedVertices::VertexTransform &other) const {
  if (_joint_index != other._joint_index) {
    return _joint_index < other._joint_index;
  }
  return _effect < other._effect;
}


