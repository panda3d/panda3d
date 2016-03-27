/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBounds3.cxx
 * @author enn0x
 * @date 2009-10-31
 */

#include "physxBounds3.h"
#include "physxManager.h"

/**
 * Returns the minimum corner of the bounding box.
 */
LPoint3f PhysxBounds3::
get_max() const {

  return PhysxManager::nxVec3_to_point3(_bounds.max);
}

/**
 * Returns the maximum corner of the bounding box.
 */
LPoint3f PhysxBounds3::
get_min() const {

  return PhysxManager::nxVec3_to_point3(_bounds.min);
}

/**
 * Returns the center of the bounding box.
 */
LPoint3f PhysxBounds3::
get_center() const {

  NxVec3 center;
  _bounds.getCenter(center);
  return PhysxManager::nxVec3_to_point3(center);
}

/**
 * Returns the extents of the bounding box.
 */
LVector3f PhysxBounds3::
get_dimensions() const {

  NxVec3 dims;
  _bounds.getDimensions(dims);
  return PhysxManager::nxVec3_to_vec3(dims);
}

/**
 * Sets the maximum corner of the bounding box.
 */
void PhysxBounds3::
set_max(LPoint3f value) {

  nassertv(!value.is_nan());

  _bounds.max = PhysxManager::point3_to_nxVec3(value);
}

/**
 * Sets the minimum corner of the bounding box.
 */
void PhysxBounds3::
set_min(LPoint3f value) {

  nassertv(!value.is_nan());

  _bounds.min = PhysxManager::point3_to_nxVec3(value);
}

/**
 * Sets this to the AABB (axis ligned bounding box) of the OBB (oriented
 * bounding box). The OBB is described by orientation, translation and half
 * dimensions.
 */
void PhysxBounds3::
bounds_of_obb(const LMatrix3f &orientation, const LPoint3f &translation, const LVector3f &half_dims) {

  nassertv(!orientation.is_nan());
  nassertv(!translation.is_nan());
  nassertv(!half_dims.is_nan());

  _bounds.boundsOfOBB(PhysxManager::mat3_to_nxMat33(orientation),
                      PhysxManager::point3_to_nxVec3(translation),
                      PhysxManager::vec3_to_nxVec3(half_dims));
}

/**
 * Sets this to the union of this and b2.
 */
void PhysxBounds3::
combine(const PhysxBounds3 &b2) {

  _bounds.combine(b2._bounds);
}

/**
 * Returns TRUE if these bounds contain the point v.
 */
bool PhysxBounds3::
contain(const LPoint3f &p) const {

  nassertr(!p.is_nan(), false);

  return _bounds.contain(PhysxManager::point3_to_nxVec3(p));
}

/**
 * Fattens the AABB in all three dimensions by the given distance.
 */
void PhysxBounds3::
fatten(float distance) {

  _bounds.fatten(distance);
}

/**
 * Expands the volume to include the point v.
 */
void PhysxBounds3::
include(const LPoint3f &p) {

  nassertv(!p.is_nan());
  _bounds.include(PhysxManager::point3_to_nxVec3(p));
}

/**
 * Returns TRUE if the intersection of this and b is is not empty.
 */
bool PhysxBounds3::
intersects(const PhysxBounds3 &b) const {

  return _bounds.intersects(b._bounds);
}

/**
 * Indicates whether the intersection of this and b is empty or not in the
 * plane orthogonal to the axis passed (X = 0, Y = 1 or Z = 2).
 */
bool PhysxBounds3::
intersects2d(const PhysxBounds3 &b, unsigned axis_to_ignore) const {

  return _bounds.intersects2D(b._bounds, axis_to_ignore);
}

/**
 * Returns TRUE if the bounding box is empty.
 */
bool PhysxBounds3::
is_empty() const {

  return _bounds.isEmpty();
}

/**
 * Scales the AABB by the given factor.
 */
void PhysxBounds3::
scale(float scale) {

  _bounds.scale(scale);
}

/**
 * Setup this AABB from minimum corner and maximum corner.
 */
void PhysxBounds3::
set(const LPoint3f &min, const LPoint3f &max) {

  nassertv(!min.is_nan());
  nassertv(!max.is_nan());

  _bounds.set(PhysxManager::point3_to_nxVec3(min),
              PhysxManager::point3_to_nxVec3(max));
}

/**
 * Setup this AABB from center point and extents vector.
 */
void PhysxBounds3::
set_center_extents(const LPoint3f &center, const LVector3f &extents) {

  nassertv(!center.is_nan());
  nassertv(!extents.is_nan());

  _bounds.setCenterExtents(PhysxManager::point3_to_nxVec3(center),
                           PhysxManager::vec3_to_nxVec3(extents));
}

/**
 * Sets empty to TRUE.
 */
void PhysxBounds3::
set_empty() {

  _bounds.setEmpty();
}

/**
 * Sets infinite bounds.
 */
void PhysxBounds3::
set_infinite() {

  _bounds.setInfinite();
}

/**
 * Transforms this volume as if it was an axis aligned bounding box, and then
 * assigns the results' bounds to this.  The orientation is applied first,
 * then the translation.
 */
void PhysxBounds3::
transform(const LMatrix3f &orientation, const LPoint3f &translation) {

  nassertv(!orientation.is_nan());
  nassertv(!translation.is_nan());

  _bounds.transform(PhysxManager::mat3_to_nxMat33(orientation),
                    PhysxManager::point3_to_nxVec3(translation));
}
