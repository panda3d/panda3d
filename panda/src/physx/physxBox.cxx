// Filename: physxBox.cxx
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

#include "physxBox.h"

#include "luse.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxBox
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxBox::
PhysxBox() {
  nBox = new NxBox();
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxBox
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxBox::
PhysxBox(const LVecBase3f & _center, const LVecBase3f & _extents, const LMatrix3f & _rot) {
  nBox = new NxBox(PhysxManager::lVecBase3_to_nxVec3(_center), PhysxManager::lVecBase3_to_nxVec3(_extents), PhysxManager::lMatrix3_to_nxMat33(_rot));
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysxBox
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxBox::
~PhysxBox() {
  delete nBox;
}

////////////////////////////////////////////////////////////////////
//     Function : get_center
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBox::
get_center() const {
  nassertr(nBox != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nBox->center);
}

////////////////////////////////////////////////////////////////////
//     Function : get_extents
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBox::
get_extents() const {
  nassertr(nBox != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nBox->extents);
}

////////////////////////////////////////////////////////////////////
//     Function : get_rot
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxBox::
get_rot() const {
  nassertr(nBox != NULL, *((LMatrix3f *)NULL));

  return PhysxManager::nxMat33_to_lMatrix3(nBox->rot);
}

////////////////////////////////////////////////////////////////////
//     Function : set_center
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBox::
set_center(LVecBase3f value) {
  nassertv(nBox != NULL);

  nBox->center = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_extents
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBox::
set_extents(LVecBase3f value) {
  nassertv(nBox != NULL);

  nBox->extents = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_rot
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBox::
set_rot(LMatrix3f value) {
  nassertv(nBox != NULL);

  nBox->rot = PhysxManager::lMatrix3_to_nxMat33(value);
}

#endif // HAVE_PHYSX

