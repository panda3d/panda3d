// Filename: physxBounds3.cxx
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

#include "physxBounds3.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::get_max
//       Access: Published
//  Description: Returns the minimum corner of the bounding box.
////////////////////////////////////////////////////////////////////
LPoint3f PhysxBounds3::
get_max() const {

  return PhysxManager::nxVec3_to_point3(_bounds.max);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::get_min
//       Access: Published
//  Description: Returns the maximum corner of the bounding box.
////////////////////////////////////////////////////////////////////
LPoint3f PhysxBounds3::
get_min() const {

  return PhysxManager::nxVec3_to_point3(_bounds.min);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::get_center
//       Access: Published
//  Description: Returns the center of the bounding box.
////////////////////////////////////////////////////////////////////
LPoint3f PhysxBounds3::
get_center() const {

  NxVec3 center;
  _bounds.getCenter(center);
  return PhysxManager::nxVec3_to_point3(center);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::get_dimensions
//       Access: Published
//  Description: Returns the extents of the bounding box.
////////////////////////////////////////////////////////////////////
LVector3f PhysxBounds3::
get_dimensions() const {

  NxVec3 dims;
  _bounds.getDimensions(dims);
  return PhysxManager::nxVec3_to_vec3(dims);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::set_max
//       Access: Published
//  Description: Sets the maximum corner of the bounding box.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
set_max(LPoint3f value) {

  nassertv(!value.is_nan());

  _bounds.max = PhysxManager::point3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::set_min
//       Access: Published
//  Description: Sets the minimum corner of the bounding box.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
set_min(LPoint3f value) {

  nassertv(!value.is_nan());

  _bounds.min = PhysxManager::point3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::bounds_of_obb
//       Access: Published
//  Description: Sets this to the AABB (axis ligned bounding box)
//               of the OBB (oriented bounding box). The OBB is
//               described by orientation, translation and half
//               dimensions.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
bounds_of_obb(const LMatrix3f &orientation, const LPoint3f &translation, const LVector3f &half_dims) {

  nassertv(!orientation.is_nan());
  nassertv(!translation.is_nan());
  nassertv(!half_dims.is_nan());

  _bounds.boundsOfOBB(PhysxManager::mat3_to_nxMat33(orientation),
                      PhysxManager::point3_to_nxVec3(translation),
                      PhysxManager::vec3_to_nxVec3(half_dims));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::combine
//       Access: Published
//  Description: Sets this to the union of this and b2. 
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
combine(const PhysxBounds3 &b2) {

  _bounds.combine(b2._bounds);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::contain
//       Access: Published
//  Description: Returns TRUE if these bounds contain the point v.
////////////////////////////////////////////////////////////////////
bool PhysxBounds3::
contain(const LPoint3f &p) const {

  nassertr(!p.is_nan(), false);

  return _bounds.contain(PhysxManager::point3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::fatten
//       Access: Published
//  Description: Fattens the AABB in all three dimensions by the
//               given distance.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
fatten(float distance) {

  _bounds.fatten(distance);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::include
//       Access: Published
//  Description: Expands the volume to include the point v.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
include(const LPoint3f &p) {

  nassertv(!p.is_nan());
  _bounds.include(PhysxManager::point3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::intersects
//       Access: Published
//  Description: Returns TRUE if the intersection of this and b is
//               is not empty.
////////////////////////////////////////////////////////////////////
bool PhysxBounds3::
intersects(const PhysxBounds3 &b) const {

  return _bounds.intersects(b._bounds);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::intersects2d
//       Access: Published
//  Description: Indicates whether the intersection of this and b
//               is empty or not in the plane orthogonal to the
//               axis passed (X = 0, Y = 1 or Z = 2). 
////////////////////////////////////////////////////////////////////
bool PhysxBounds3::
intersects2d(const PhysxBounds3 &b, unsigned axis_to_ignore) const {

  return _bounds.intersects2D(b._bounds, axis_to_ignore);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::is_empty
//       Access: Published
//  Description: Returns TRUE if the bounding box is empty.
////////////////////////////////////////////////////////////////////
bool PhysxBounds3::
is_empty() const {

  return _bounds.isEmpty();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::scale
//       Access: Published
//  Description: Scales the AABB by the given factor.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
scale(float scale) {

  _bounds.scale(scale);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::set
//       Access: Published
//  Description: Setup this AABB from minimum corner and maximum
//               corner.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
set(const LPoint3f &min, const LPoint3f &max) {

  nassertv(!min.is_nan());
  nassertv(!max.is_nan());

  _bounds.set(PhysxManager::point3_to_nxVec3(min),
              PhysxManager::point3_to_nxVec3(max));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::set_center_extents
//       Access: Published
//  Description: Setup this AABB from center point and extents
//               vector.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
set_center_extents(const LPoint3f &center, const LVector3f &extents) {

  nassertv(!center.is_nan());
  nassertv(!extents.is_nan());

  _bounds.setCenterExtents(PhysxManager::point3_to_nxVec3(center),
                           PhysxManager::vec3_to_nxVec3(extents));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::set_empty
//       Access: Published
//  Description: Sets empty to TRUE.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
set_empty() {

  _bounds.setEmpty();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::set_infinite
//       Access: Published
//  Description: Sets infinite bounds.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
set_infinite() {

  _bounds.setInfinite();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBounds3::transform
//       Access: Published
//  Description: Transforms this volume as if it was an axis aligned
//               bounding box, and then assigns the results' bounds
//               to this. The orientation is applied first, then the
//               translation.
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
transform(const LMatrix3f &orientation, const LPoint3f &translation) {

  nassertv(!orientation.is_nan());
  nassertv(!translation.is_nan());

  _bounds.transform(PhysxManager::mat3_to_nxMat33(orientation),
                    PhysxManager::point3_to_nxVec3(translation));
}

