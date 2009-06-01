// Filename: physxBounds3.cxx
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

#ifdef HAVE_PHYSX

#include "physxBounds3.h"

#include "luse.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxBounds3
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxBounds3::
PhysxBounds3() {
  nBounds3 = new NxBounds3();
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysxBounds3
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxBounds3::
~PhysxBounds3() {
  delete nBounds3;
}

////////////////////////////////////////////////////////////////////
//     Function : get_max
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBounds3::
get_max() const {
  nassertr(nBounds3 != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nBounds3->max);
}

////////////////////////////////////////////////////////////////////
//     Function : get_min
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBounds3::
get_min() const {
  nassertr(nBounds3 != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nBounds3->min);
}

////////////////////////////////////////////////////////////////////
//     Function : set_max
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
set_max(LVecBase3f value) {
  nassertv(nBounds3 != NULL);

  nBounds3->max = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_min
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBounds3::
set_min(LVecBase3f value) {
  nassertv(nBounds3 != NULL);

  nBounds3->min = PhysxManager::lVecBase3_to_nxVec3(value);
}

#endif // HAVE_PHYSX

