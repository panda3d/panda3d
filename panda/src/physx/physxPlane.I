// Filename: physxPlane.I
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
//     Function : belongs
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxPlane::
belongs(const LVecBase3f & p) const {
  nassertr(nPlane != NULL, false);

  return nPlane->belongs(PhysxManager::lVecBase3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function : distance
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE float PhysxPlane::
distance(const LVecBase3f & p) const {
  nassertr(nPlane != NULL, -1.0f);

  return nPlane->distance(PhysxManager::lVecBase3_to_nxVec3(p));
}

////////////////////////////////////////////////////////////////////
//     Function : inverse_transform
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxPlane::
inverse_transform(const LMatrix4f & transform, PhysxPlane & transformed) const {
  nassertv(nPlane != NULL);

  nPlane->inverseTransform(PhysxManager::lMatrix4_to_nxMat34(transform), *(transformed.nPlane));
}

////////////////////////////////////////////////////////////////////
//     Function : normalize
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxPlane::
normalize() {
  nassertv(nPlane != NULL);

  nPlane->normalize();
}

////////////////////////////////////////////////////////////////////
//     Function : transform
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxPlane::
transform(const LMatrix4f & transform, PhysxPlane & transformed) const {
  nassertv(nPlane != NULL);

  nPlane->transform(PhysxManager::lMatrix4_to_nxMat34(transform), *(transformed.nPlane));
}

////////////////////////////////////////////////////////////////////
//     Function : get_d
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE float PhysxPlane::
get_d() const {
  nassertr(nPlane != NULL, -1.0f);

  return nPlane->d;
}

////////////////////////////////////////////////////////////////////
//     Function : set_d
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxPlane::
set_d(float value) {
  nassertv(nPlane != NULL);

  nPlane->d = value;
}


