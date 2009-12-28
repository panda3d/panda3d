// Filename: physxSphere.cxx
// Created by:  enn0x (31Oct09)
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

#include "physxSphere.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::is_valid
//       Access: Published
//  Description: Returns TRUE if this sphere is valid.
////////////////////////////////////////////////////////////////////
bool PhysxSphere::
is_valid() const {

  return _sphere.IsValid();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::contains
//       Access: Published
//  Description: Tests if a point is contained within the sphere.
////////////////////////////////////////////////////////////////////
bool PhysxSphere::
contains(const LPoint3f &p) const {

  nassertr(!p.is_nan(), false);

  return _sphere.Contains(PhysxManager::vec3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::contains
//       Access: Published
//  Description: Tests if a sphere is contained within the sphere.
////////////////////////////////////////////////////////////////////
bool PhysxSphere::
contains(const PhysxSphere &sphere) const {

  return _sphere.Contains(sphere._sphere);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::contains
//       Access: Published
//  Description: Tests if an axis aligned box is contained within
//               the sphere. The axis aligned box is defined by the
//               minimum corner and the maximum corner.
////////////////////////////////////////////////////////////////////
bool PhysxSphere::
contains(const LPoint3f &min, const LPoint3f &max) const {

  nassertr(!min.is_nan(), false);
  nassertr(!max.is_nan(), false);

  return _sphere.Contains(PhysxManager::vec3_to_nxVec3(min), 
                          PhysxManager::vec3_to_nxVec3(max));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::intersect
//       Access: Published
//  Description: Tests if the sphere intersects another sphere.
//               Returns TRUE if the spheres overlap.
////////////////////////////////////////////////////////////////////
bool PhysxSphere::
intersect(const PhysxSphere &sphere) const {

  return _sphere.Intersect(sphere._sphere);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::get_center
//       Access: Published
//  Description: Returns the center of the sphere.
////////////////////////////////////////////////////////////////////
LPoint3f PhysxSphere::
get_center() const {

  return PhysxManager::nxVec3_to_vec3(_sphere.center);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::set_center
//       Access: Published
//  Description: Sets the center of the sphere.
////////////////////////////////////////////////////////////////////
void PhysxSphere::
set_center(LPoint3f center) {

  nassertv(!center.is_nan());

  _sphere.center = PhysxManager::vec3_to_nxVec3(center);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::get_radius
//       Access: Published
//  Description: Returns the sphere's radius.
////////////////////////////////////////////////////////////////////
float PhysxSphere::
get_radius() const {

  return _sphere.radius;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphere::set_radius
//       Access: Published
//  Description: Sets the sphere's radius.
////////////////////////////////////////////////////////////////////
void PhysxSphere::
set_radius(float radius) {

  _sphere.radius = radius;
}

