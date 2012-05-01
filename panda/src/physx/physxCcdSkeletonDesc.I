// Filename: physxCcdSkeletonDesc.I
// Created by:  enn0x (01May12)
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
//     Function: PhysxCcdSkeletonDesc::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCcdSkeletonDesc::
PhysxCcdSkeletonDesc() {

  _desc.flags = 0;
  _desc.pointStrideBytes = sizeof(NxVec3);
  _desc.triangleStrideBytes = 3*sizeof(NxU32);
  _desc.points = NULL;
  _desc.triangles = NULL;

  _vertices = NULL;
  _triangles = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeletonDesc::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCcdSkeletonDesc::
~PhysxCcdSkeletonDesc() {

  if (_vertices) {
    delete [] _vertices;
  }

  if (_triangles) {
    delete [] _triangles;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeletonDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxCcdSkeletonDesc::
is_valid() const {

  return _desc.isValid();
}

