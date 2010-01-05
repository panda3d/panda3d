// Filename: physxTriangleMeshShapeDesc.cxx
// Created by:  enn0x (14Oct09)
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

#include "physxTriangleMeshShapeDesc.h"
#include "physxTriangleMesh.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMeshShapeDesc::set_mesh
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxTriangleMeshShapeDesc::
set_mesh(PhysxTriangleMesh *mesh) {

  _desc.meshData = mesh->ptr();
}


