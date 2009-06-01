// Filename: physxSegment.I
// Created by:  pratt (Apr 7, 2006)
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
//     Function : compute_direction
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxSegment::
compute_direction(LVecBase3f & dir) const {
  nassertv(nSegment != NULL);

  nSegment->computeDirection(PhysxManager::lVecBase3_to_nxVec3(dir));
}

////////////////////////////////////////////////////////////////////
//     Function : compute_length
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE float PhysxSegment::
compute_length() const {
  nassertr(nSegment != NULL, -1.0f);

  return nSegment->computeLength();
}

////////////////////////////////////////////////////////////////////
//     Function : compute_point
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxSegment::
compute_point(LVecBase3f & pt, float t) const {
  nassertv(nSegment != NULL);

  nSegment->computePoint(PhysxManager::lVecBase3_to_nxVec3(pt), t);
}

////////////////////////////////////////////////////////////////////
//     Function : compute_square_length
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE float PhysxSegment::
compute_square_length() const {
  nassertr(nSegment != NULL, -1.0f);

  return nSegment->computeSquareLength();
}

////////////////////////////////////////////////////////////////////
//     Function : set_origin_direction
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxSegment::
set_origin_direction(const LVecBase3f & origin, const LVecBase3f & direction) {
  nassertv(nSegment != NULL);

  nSegment->setOriginDirection(PhysxManager::lVecBase3_to_nxVec3(origin), PhysxManager::lVecBase3_to_nxVec3(direction));
}

