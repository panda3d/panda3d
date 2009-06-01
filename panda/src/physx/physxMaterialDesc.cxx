// Filename: physxMaterialDesc.cxx
// Created by:  pratt (Jul 9, 2006)
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

#include "physxMaterialDesc.h"

#include "luse.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxMaterialDesc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxMaterialDesc::
PhysxMaterialDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function : get_dir_of_anisotropy
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxMaterialDesc::
get_dir_of_anisotropy() const {
  return PhysxManager::nxVec3_to_lVecBase3(nMaterialDesc.dirOfAnisotropy);
}

////////////////////////////////////////////////////////////////////
//     Function : get_friction_combine_mode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxCombineMode PhysxMaterialDesc::
get_friction_combine_mode() const {
  return (PhysxCombineMode)nMaterialDesc.frictionCombineMode;
}

////////////////////////////////////////////////////////////////////
//     Function : get_restitution_combine_mode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxCombineMode PhysxMaterialDesc::
get_restitution_combine_mode() const {
  return (PhysxCombineMode)nMaterialDesc.restitutionCombineMode;
}

////////////////////////////////////////////////////////////////////
//     Function : set_dir_of_anisotropy
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_dir_of_anisotropy(LVecBase3f value) {
  nMaterialDesc.dirOfAnisotropy = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_friction_combine_mode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_friction_combine_mode(PhysxCombineMode value) {
  nMaterialDesc.frictionCombineMode = (NxCombineMode)value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_restitution_combine_mode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_restitution_combine_mode(PhysxCombineMode value) {
  nMaterialDesc.restitutionCombineMode = (NxCombineMode)value;
}

#endif // HAVE_PHYSX

