/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSegment.cxx
 * @author enn0x
 * @date 2009-10-31
 */

#include "physxSegment.h"
#include "physxManager.h"

/**

 */
PhysxSegment::
PhysxSegment(const LPoint3f &p0, const LPoint3f &p1) {

  _segment.p0 = PhysxManager::point3_to_nxVec3(p0);
  _segment.p1 = PhysxManager::point3_to_nxVec3(p1);
}

/**
 * Returns the start point of the segment.
 */
LPoint3f PhysxSegment::
get_p0() const {

  return PhysxManager::nxVec3_to_vec3(_segment.p0);
}

/**
 * Sets the start point of the segment.
 */
void PhysxSegment::
set_p0(LPoint3f p) {

  nassertv_always(!p.is_nan());

  _segment.p0 = PhysxManager::vec3_to_nxVec3(p);
}

/**
 * Returns the end point of the segment.
 */
LPoint3f PhysxSegment::
get_p1() const {

  return PhysxManager::nxVec3_to_vec3(_segment.p1);
}

/**
 * Sets the end point of the segment.
 */
void PhysxSegment::
set_p1(LPoint3f p) {

  nassertv_always(!p.is_nan());

  _segment.p1 = PhysxManager::vec3_to_nxVec3(p);
}

/**
 * Returns the start point of the segment.
 */
LPoint3f PhysxSegment::
get_origin() const {

  return PhysxManager::nxVec3_to_point3(_segment.getOrigin());
}

/**
 * Returns the direction vector from the segment's start point to it's end
 * point.
 */
void PhysxSegment::
compute_direction(LPoint3f &dir) const {

  nassertv(!dir.is_nan());

  NxVec3 nDir = PhysxManager::point3_to_nxVec3(dir);
  _segment.computeDirection(nDir);
  PhysxManager::update_point3_from_nxVec3(dir, nDir);
}

/**
 * Returns the distance from the segment's start point to it's end point.
 */
float PhysxSegment::
compute_length() const {

  return _segment.computeLength();
}

/**
 * Computes a point on the segment.
 */
void PhysxSegment::
compute_point(LPoint3f &p, float t) const {

  nassertv(!p.is_nan());

  NxVec3 nP = PhysxManager::point3_to_nxVec3(p);
  _segment.computePoint(nP, t);
  PhysxManager::update_point3_from_nxVec3(p, nP);
}

/**
 * Returns the square distance from the segment's start point to it's end point.
 */
float PhysxSegment::
compute_square_length() const {

  return _segment.computeSquareLength();
}

/**
 * Setup this segment from origin (start point) and direction vector.
 */
void PhysxSegment::
set_origin_direction(const LPoint3f &origin, const LVector3f &direction) {

  nassertv_always(!origin.is_nan());
  nassertv_always(!direction.is_nan());

  _segment.setOriginDirection(PhysxManager::point3_to_nxVec3(origin),
                              PhysxManager::vec3_to_nxVec3(direction));
}
