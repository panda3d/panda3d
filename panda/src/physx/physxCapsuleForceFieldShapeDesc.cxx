// Filename: physxCapsuleForceFieldShapeDesc.cxx
// Created by:  enn0x (06Nov09)
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

#include "physxCapsuleForceFieldShapeDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleForceFieldShapeDesc::set_radius
//       Access: Published
//  Description: Sets the radius of the capsule's hemispherical
//               ends and its trunk.
////////////////////////////////////////////////////////////////////
void PhysxCapsuleForceFieldShapeDesc::
set_radius(float radius) {

  _desc.radius = radius;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleForceFieldShapeDesc::set_height
//       Access: Published
//  Description: Sets the distance between the two hemispherical
//               ends of the capsule.
////////////////////////////////////////////////////////////////////
void PhysxCapsuleForceFieldShapeDesc::
set_height(float height) {

  _desc.height = height;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleForceFieldShapeDesc::get_radius
//       Access: Published
//  Description: The radius of the capsule's hemispherical ends
//               and its trunk.
////////////////////////////////////////////////////////////////////
float PhysxCapsuleForceFieldShapeDesc::
get_radius() const {

  return _desc.radius;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleForceFieldShapeDesc::get_height
//       Access: Published
//  Description: The distance between the two hemispherical ends
//               of the capsule.
////////////////////////////////////////////////////////////////////
float PhysxCapsuleForceFieldShapeDesc::
get_height() const {

  return _desc.height;
}

