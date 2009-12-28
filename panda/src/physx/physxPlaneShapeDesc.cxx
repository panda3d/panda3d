// Filename: physxPlaneShapeDesc.cxx
// Created by:  enn0x (08Sep09)
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

#include "physxPlaneShapeDesc.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlaneShapeDesc::set_plane
//       Access: Published
//  Description: Sets the parameters of the plane equation.
//               normal: Plane normal.
//               d: The distance from the origin. 
////////////////////////////////////////////////////////////////////
void PhysxPlaneShapeDesc::
set_plane(const LVector3f &normal, float d) {

  nassertv(!normal.is_nan());

  _desc.normal = PhysxManager::vec3_to_nxVec3(normal);
  _desc.d = d;
}

