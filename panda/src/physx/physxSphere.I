// Filename: physxSphere.I
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
//     Function : contains
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxSphere::
contains(const LVecBase3f & min, const LVecBase3f & max) const {
  nassertr(nSphere != NULL, false);

  return nSphere->Contains(PhysxManager::lVecBase3_to_nxVec3(min), PhysxManager::lVecBase3_to_nxVec3(max));
}

////////////////////////////////////////////////////////////////////
//     Function : intersect
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxSphere::
intersect(const PhysxSphere & sphere) const {
  nassertr(nSphere != NULL, false);

  return nSphere->Intersect(*(sphere.nSphere));
}

////////////////////////////////////////////////////////////////////
//     Function : is_valid
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxSphere::
is_valid() const {
  nassertr(nSphere != NULL, false);

  return nSphere->IsValid();
}

////////////////////////////////////////////////////////////////////
//     Function : get_radius
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE float PhysxSphere::
get_radius() const {
  nassertr(nSphere != NULL, -1.0f);

  return nSphere->radius;
}

////////////////////////////////////////////////////////////////////
//     Function : set_radius
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxSphere::
set_radius(float value) {
  nassertv(nSphere != NULL);

  nSphere->radius = value;
}


