// Filename: physxSoftBodyMeshDesc.I
// Created by:  enn0x (12Sep10)
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
//     Function: PhysxSoftBodyMeshDesc::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSoftBodyMeshDesc::
PhysxSoftBodyMeshDesc() {

  _desc.flags = 0;
  _desc.vertexStrideBytes = sizeof(NxVec3);
  _desc.tetrahedronStrideBytes = 4*sizeof(NxU32);
  _desc.vertices = NULL;
  _desc.tetrahedra = NULL;

  _vertices = NULL;
  _tetrahedra = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMeshDesc::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSoftBodyMeshDesc::
~PhysxSoftBodyMeshDesc() {

  if (_vertices) {
    delete [] _vertices;
  }

  if (_tetrahedra) {
    delete [] _tetrahedra;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMeshDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxSoftBodyMeshDesc::
is_valid() const {

  return _desc.isValid();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMeshDesc::get_desc
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const NxSoftBodyMeshDesc &PhysxSoftBodyMeshDesc::
get_desc() const {

  return _desc;
}

