// Filename: physxPlane.cxx
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

#ifdef HAVE_PHYSX

#include "physxPlane.h"

#include "luse.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxPlane
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxPlane::
PhysxPlane(const PhysxPlane & plane) {

}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysxPlane
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxPlane::
~PhysxPlane() {

}

////////////////////////////////////////////////////////////////////
//     Function : point_in_plane
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxPlane::
point_in_plane() const {
  nassertr(nPlane != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nPlane->pointInPlane());
}

////////////////////////////////////////////////////////////////////
//     Function : project
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxPlane::
project(const LVecBase3f & p) const {
  nassertr(nPlane != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nPlane->project(PhysxManager::lVecBase3_to_nxVec3(p)));
}

////////////////////////////////////////////////////////////////////
//     Function : set
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxPlane & PhysxPlane::
set(const LVecBase3f & p0, const LVecBase3f & p1, const LVecBase3f & p2) {
//  return *((PhysxPlane *)(nPlane->set(PhysxManager::lVecBase3_to_nxVec3(p0), PhysxManager::lVecBase3_to_nxVec3(p1), PhysxManager::lVecBase3_to_nxVec3(p2)).userData));
  throw "Not Implemented";
}

////////////////////////////////////////////////////////////////////
//     Function : zero
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxPlane & PhysxPlane::
zero() {
//  return *((PhysxPlane *)(nPlane->zero().userData));
  throw "Not Implemented";
}

////////////////////////////////////////////////////////////////////
//     Function : get_normal
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxPlane::
get_normal() const {
  nassertr(nPlane != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nPlane->normal);
}

////////////////////////////////////////////////////////////////////
//     Function : set_normal
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxPlane::
set_normal(LVecBase3f value) {
  nassertv(nPlane != NULL);

  nPlane->normal = PhysxManager::lVecBase3_to_nxVec3(value);
}

#endif // HAVE_PHYSX

