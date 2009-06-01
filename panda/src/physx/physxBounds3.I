// Filename: physxBounds3.I
// Created by:  pratt (Dec 12, 2007)
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

////////////////////////////////////////////////////////////////////
//     Function : bounds_of_obb
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
bounds_of_obb(const LMatrix3f & orientation, const LVecBase3f & translation, const LVecBase3f & half_dims) {
  nassertv(nBounds3 != NULL);

  nBounds3->boundsOfOBB(PhysxManager::lMatrix3_to_nxMat33(orientation), PhysxManager::lVecBase3_to_nxVec3(translation), PhysxManager::lVecBase3_to_nxVec3(half_dims));
}

////////////////////////////////////////////////////////////////////
//     Function : combine
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
combine(const PhysxBounds3 & b2) {
  nassertv(nBounds3 != NULL);

  nBounds3->combine(*(b2.nBounds3));
}

////////////////////////////////////////////////////////////////////
//     Function : contain
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBounds3::
contain(const LVecBase3f & v) const {
  nassertr(nBounds3 != NULL, false);

  return nBounds3->contain(PhysxManager::lVecBase3_to_nxVec3(v));
}

////////////////////////////////////////////////////////////////////
//     Function : fatten
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
fatten(float distance) {
  nassertv(nBounds3 != NULL);

  nBounds3->fatten(distance);
}

////////////////////////////////////////////////////////////////////
//     Function : get_dimensions
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
get_dimensions(LVecBase3f & dims) const {
  nassertv(nBounds3 != NULL);

  nBounds3->getDimensions(PhysxManager::lVecBase3_to_nxVec3(dims));
}

////////////////////////////////////////////////////////////////////
//     Function : include
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
include(const LVecBase3f & v) {
  nassertv(nBounds3 != NULL);

  nBounds3->include(PhysxManager::lVecBase3_to_nxVec3(v));
}

////////////////////////////////////////////////////////////////////
//     Function : intersects
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBounds3::
intersects(const PhysxBounds3 & b) const {
  nassertr(nBounds3 != NULL, false);

  return nBounds3->intersects(*(b.nBounds3));
}

////////////////////////////////////////////////////////////////////
//     Function : intersects2d
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBounds3::
intersects2d(const PhysxBounds3 & b, unsigned axis_to_ignore) const {
  nassertr(nBounds3 != NULL, false);

  return nBounds3->intersects2D(*(b.nBounds3), axis_to_ignore);
}

////////////////////////////////////////////////////////////////////
//     Function : is_empty
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBounds3::
is_empty() const {
  nassertr(nBounds3 != NULL, false);

  return nBounds3->isEmpty();
}

////////////////////////////////////////////////////////////////////
//     Function : scale
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
scale(float scale) {
  nassertv(nBounds3 != NULL);

  nBounds3->scale(scale);
}

////////////////////////////////////////////////////////////////////
//     Function : set
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
set(const LVecBase3f & min, const LVecBase3f & max) {
  nassertv(nBounds3 != NULL);

  nBounds3->set(PhysxManager::lVecBase3_to_nxVec3(min), PhysxManager::lVecBase3_to_nxVec3(max));
}

////////////////////////////////////////////////////////////////////
//     Function : set_center_extents
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
set_center_extents(const LVecBase3f & c, const LVecBase3f & e) {
  nassertv(nBounds3 != NULL);

  nBounds3->setCenterExtents(PhysxManager::lVecBase3_to_nxVec3(c), PhysxManager::lVecBase3_to_nxVec3(e));
}

////////////////////////////////////////////////////////////////////
//     Function : set_empty
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
set_empty() {
  nassertv(nBounds3 != NULL);

  nBounds3->setEmpty();
}

////////////////////////////////////////////////////////////////////
//     Function : set_infinite
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
set_infinite() {
  nassertv(nBounds3 != NULL);

  nBounds3->setInfinite();
}

////////////////////////////////////////////////////////////////////
//     Function : transform
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxBounds3::
transform(const LMatrix3f & orientation, const LVecBase3f & translation) {
  nassertv(nBounds3 != NULL);

  nBounds3->transform(PhysxManager::lMatrix3_to_nxMat33(orientation), PhysxManager::lVecBase3_to_nxVec3(translation));
}

