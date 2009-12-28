// Filename: physxSphereShapeDesc.cxx
// Created by:  enn0x (11Sep09)
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

#include "physxSphereShapeDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereShapeDesc::set_radius
//       Access: Published
//  Description: Radius of shape. Must be positive.
////////////////////////////////////////////////////////////////////
void PhysxSphereShapeDesc::
set_radius(float radius) {

  _desc.radius = radius;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereShapeDesc::get_radius
//       Access: Published
//  Description: Radius of shape.
////////////////////////////////////////////////////////////////////
float PhysxSphereShapeDesc::
get_radius() const {

  return _desc.radius;
}

