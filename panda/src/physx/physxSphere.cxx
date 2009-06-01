// Filename: physxSphere.cxx
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

#include "physxSphere.h"

#include "luse.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxSphere
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSphere::
PhysxSphere(const PhysxSphere & sphere) {

}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysxSphere
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSphere::
~PhysxSphere() {

}

////////////////////////////////////////////////////////////////////
//     Function : get_center
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxSphere::
get_center() const {
  nassertr(nSphere != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nSphere->center);
}

////////////////////////////////////////////////////////////////////
//     Function : set_center
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSphere::
set_center(LVecBase3f value) {
  nassertv(nSphere != NULL);

  nSphere->center = PhysxManager::lVecBase3_to_nxVec3(value);
}

#endif // HAVE_PHYSX


