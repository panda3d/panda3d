// Filename: physxConvexMeshDesc.I
// Created by:  enn0x (11Oct09)
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
//     Function: PhysxConvexMeshDesc::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxConvexMeshDesc::
PhysxConvexMeshDesc() {

  _desc.flags = NX_CF_COMPUTE_CONVEX;
  _desc.pointStrideBytes = sizeof(NxVec3);
  _desc.points = NULL;

  _vertices = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMeshDesc::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxConvexMeshDesc::
~PhysxConvexMeshDesc() {

  if (_vertices) {
    delete [] _vertices;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMeshDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxConvexMeshDesc::
is_valid() const {

  return _desc.isValid();
}

