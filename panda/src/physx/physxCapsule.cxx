/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCapsule.cxx
 * @author enn0x
 * @date 2009-10-31
 */

#include "physxCapsule.h"
#include "physxManager.h"

/**
 * Returns the capsule's radius.
 */
float PhysxCapsule::
get_radius() const {

  return _capsule.radius;
}

/**
 * Sets the capsule's radius.
 */
void PhysxCapsule::
set_radius(float radius) {

  _capsule.radius = radius;
}

/**
 * Returns the start point of the segment.
 */
LPoint3f PhysxCapsule::
get_p0() const {

  return PhysxManager::nxVec3_to_vec3(_capsule.p0);
}

/**
 * Sets the start point of the segment.
 */
void PhysxCapsule::
set_p0(LPoint3f p) {

  nassertv(!p.is_nan());

  _capsule.p0 = PhysxManager::vec3_to_nxVec3(p);
}

/**
 * Returns the end point of the segment.
 */
LPoint3f PhysxCapsule::
get_p1() const {

  return PhysxManager::nxVec3_to_vec3(_capsule.p1);
}

/**
 * Sets the end point of the segment.
 */
void PhysxCapsule::
set_p1(LPoint3f p) {

  nassertv(!p.is_nan());

  _capsule.p1 = PhysxManager::vec3_to_nxVec3(p);
}

/**
 * Returns the start point of the segment.
 */
LPoint3f PhysxCapsule::
get_origin() const {

  return PhysxManager::nxVec3_to_point3(_capsule.getOrigin());
}

/**
 * Returns the direction vector from the segment's start point to it's end
 * point.
 */
void PhysxCapsule::
compute_direction(LPoint3f &dir) const {

  nassertv(!dir.is_nan());

  NxVec3 nDir = PhysxManager::point3_to_nxVec3(dir);
  _capsule.computeDirection(nDir);
  PhysxManager::update_point3_from_nxVec3(dir, nDir);
}

/**
 * Returns the distance from the segment's start point to it's end point.
 */
float PhysxCapsule::
compute_length() const {

  return _capsule.computeLength();
}

/**
 * Computes a point on the segment.
 */
void PhysxCapsule::
compute_point(LPoint3f &p, float t) const {

  nassertv(!p.is_nan());

  NxVec3 nP = PhysxManager::point3_to_nxVec3(p);
  _capsule.computePoint(nP, t);
  PhysxManager::update_point3_from_nxVec3(p, nP);
}

/**
 * Returns the square distance from the segment's start point to it's end point.
 */
float PhysxCapsule::
compute_square_length() const {

  return _capsule.computeSquareLength();
}

/**
 * Setup this capsule from origin (start point) and direction vector.
 */
void PhysxCapsule::
set_origin_direction(const LPoint3f &origin, const LVector3f &direction) {

  nassertv(!origin.is_nan());
  nassertv(!direction.is_nan());

  _capsule.setOriginDirection(PhysxManager::point3_to_nxVec3(origin),
                              PhysxManager::vec3_to_nxVec3(direction));
}
