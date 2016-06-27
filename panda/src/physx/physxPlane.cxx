/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxPlane.cxx
 * @author enn0x
 * @date 2009-10-31
 */

#include "physxPlane.h"
#include "physxManager.h"

/**
 *
 */
float PhysxPlane::
distance(const LPoint3f &p) const {

  nassertr(!p.is_nan(), 0.0f);

  return _plane.distance(PhysxManager::point3_to_nxVec3(p));
}

/**
 *
 */
bool PhysxPlane::
belongs(const LPoint3f &p) const {

  nassertr(!p.is_nan(), false);

  return _plane.belongs(PhysxManager::point3_to_nxVec3(p));
}

/**
 *
 */
LPoint3f PhysxPlane::
point_in_plane() const {

  return PhysxManager::nxVec3_to_point3(_plane.pointInPlane());
}

/**
 *
 */
LPoint3f PhysxPlane::
project(const LPoint3f &p) const {

  nassertr(!p.is_nan(), LPoint3f::zero());

  return PhysxManager::nxVec3_to_point3(_plane.project(PhysxManager::point3_to_nxVec3(p)));
}

/**
 *
 */
void PhysxPlane::
inverse_transform(const LMatrix4f &transform, PhysxPlane &transformed) const {

  nassertv(!transform.is_nan());

  _plane.inverseTransform(PhysxManager::mat4_to_nxMat34(transform), transformed._plane);
}

/**
 *
 */
void PhysxPlane::
normalize() {

  _plane.normalize();
}

/**
 *
 */
void PhysxPlane::
transform(const LMatrix4f &transform, PhysxPlane &transformed) const {

  nassertv(!transform.is_nan());

  _plane.transform(PhysxManager::mat4_to_nxMat34(transform), transformed._plane);
}

/**
 *
 */
float PhysxPlane::
get_d() const {

  return _plane.d;
}

/**
 *
 */
void PhysxPlane::
set_d(float value) {

  _plane.d = value;
}

/**
 *
 */
LVector3f PhysxPlane::
get_normal() const {

  return PhysxManager::nxVec3_to_vec3(_plane.normal);
}

/**
 *
 */
void PhysxPlane::
set_normal(LVector3f normal) {

  nassertv(!normal.is_nan());

  _plane.normal = PhysxManager::vec3_to_nxVec3(normal);
}

/**
 *
 */
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

/**
 *
 */
PhysxPlane PhysxPlane::
zero() {

  PhysxPlane plane;
  plane._plane = _plane.zero();
  return plane;
}
