// Filename: physxPlane.cxx
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

#include "physxPlane.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::distance
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float PhysxPlane::
distance(const LPoint3f &p) const {

  nassertr(!p.is_nan(), 0.0f);

  return _plane.distance(PhysxManager::point3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::belongs
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool PhysxPlane::
belongs(const LPoint3f &p) const {

  nassertr(!p.is_nan(), false);

  return _plane.belongs(PhysxManager::point3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::point_in_plane
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f PhysxPlane::
point_in_plane() const {

  return PhysxManager::nxVec3_to_point3(_plane.pointInPlane());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::project
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f PhysxPlane::
project(const LPoint3f &p) const {

  nassertr(!p.is_nan(), LPoint3f::zero());

  return PhysxManager::nxVec3_to_point3(_plane.project(PhysxManager::point3_to_nxVec3(p)));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::inverse_transform
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxPlane::
inverse_transform(const LMatrix4f &transform, PhysxPlane &transformed) const {

  nassertv(!transform.is_nan());

  _plane.inverseTransform(PhysxManager::mat4_to_nxMat34(transform), transformed._plane);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::normalize
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxPlane::
normalize() {

  _plane.normalize();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::transform
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxPlane::
transform(const LMatrix4f &transform, PhysxPlane &transformed) const {

  nassertv(!transform.is_nan());

  _plane.transform(PhysxManager::mat4_to_nxMat34(transform), transformed._plane);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::get_d
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float PhysxPlane::
get_d() const {

  return _plane.d;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::set_d
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxPlane::
set_d(float value) {

  _plane.d = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::get_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f PhysxPlane::
get_normal() const {

  return PhysxManager::nxVec3_to_vec3(_plane.normal);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::set_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxPlane::
set_normal(LVector3f normal) {

  nassertv(!normal.is_nan());

  _plane.normal = PhysxManager::vec3_to_nxVec3(normal);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::set
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PhysxPlane PhysxPlane::
set(const LPoint3f &p0, const LPoint3f &p1, const LPoint3f &p2) {

  PhysxPlane plane;

  nassertr(!p0.is_nan(), plane);
  nassertr(!p1.is_nan(), plane);
  nassertr(!p2.is_nan(), plane);

  plane._plane = _plane.set(PhysxManager::point3_to_nxVec3(p0),
                            PhysxManager::point3_to_nxVec3(p1),
                            PhysxManager::point3_to_nxVec3(p2));
  return plane;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPlane::zero
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PhysxPlane PhysxPlane::
zero() {

  PhysxPlane plane;
  plane._plane = _plane.zero();
  return plane;
}

