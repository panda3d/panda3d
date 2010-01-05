// Filename: physxConvexShapeDesc.cxx
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

#include "physxConvexShapeDesc.h"
#include "physxConvexMesh.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexShapeDesc::set_mesh
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxConvexShapeDesc::
set_mesh(PhysxConvexMesh *mesh) {

  _desc.meshData = mesh->ptr();
}


