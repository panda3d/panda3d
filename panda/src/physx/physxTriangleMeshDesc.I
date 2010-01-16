// Filename: physxTriangleMeshDesc.I
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
//     Function: PhysxTriangleMeshDesc::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxTriangleMeshDesc::
PhysxTriangleMeshDesc() {

  _desc.flags = 0;
  _desc.pointStrideBytes = sizeof(NxVec3);
  _desc.triangleStrideBytes = 3*sizeof(NxU32);
  _desc.materialIndexStride = sizeof(NxMaterialIndex);
  _desc.points = NULL;
  _desc.triangles = NULL;
  _desc.materialIndices = NULL;

  _vertices = NULL;
  _triangles = NULL;
  _materials = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMeshDesc::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxTriangleMeshDesc::
~PhysxTriangleMeshDesc() {

  if (_vertices) {
    delete [] _vertices;
  }

  if (_triangles) {
    delete [] _triangles;
  }

  if (_materials) {
    delete [] _materials;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMeshDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxTriangleMeshDesc::
is_valid() const {

  return _desc.isValid();
}

