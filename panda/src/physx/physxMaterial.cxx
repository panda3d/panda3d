// Filename: physxMaterial.cxx
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

#include "physxMaterial.h"

#include "luse.h"
#include "physxMaterialDesc.h"
#include "physxScene.h"


////////////////////////////////////////////////////////////////////
//     Function : get_dir_of_anisotropy
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxMaterial::
get_dir_of_anisotropy() const {
  nassertr(nMaterial != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nMaterial->getDirOfAnisotropy());
}

////////////////////////////////////////////////////////////////////
//     Function : get_dynamic_friction
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_dynamic_friction() const {
  nassertr(nMaterial != NULL, -1.0f);

  return nMaterial->getDynamicFriction();
}

////////////////////////////////////////////////////////////////////
//     Function : get_dynamic_friction_v
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_dynamic_friction_v() const {
  nassertr(nMaterial != NULL, -1.0f);

  return nMaterial->getDynamicFrictionV();
}

////////////////////////////////////////////////////////////////////
//     Function : get_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxMaterial::
get_flags() const {
  nassertr(nMaterial != NULL, -1);

  return nMaterial->getFlags();
}

////////////////////////////////////////////////////////////////////
//     Function : get_friction_combine_mode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxCombineMode PhysxMaterial::
get_friction_combine_mode() const {
  return (PhysxCombineMode)nMaterial->getFrictionCombineMode();
}

////////////////////////////////////////////////////////////////////
//     Function : get_material_index
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned short PhysxMaterial::
get_material_index() {
  nassertr(nMaterial != NULL, -1);

  return nMaterial->getMaterialIndex();
}

////////////////////////////////////////////////////////////////////
//     Function : get_restitution
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_restitution() const {
  nassertr(nMaterial != NULL, -1.0f);

  return nMaterial->getRestitution();
}

////////////////////////////////////////////////////////////////////
//     Function : get_restitution_combine_mode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxCombineMode PhysxMaterial::
get_restitution_combine_mode() const {
  return (PhysxCombineMode)nMaterial->getRestitutionCombineMode();
}

////////////////////////////////////////////////////////////////////
//     Function : get_scene
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxScene & PhysxMaterial::
get_scene() const {
  nassertr(nMaterial != NULL, *((PhysxScene *)NULL));

  return *((PhysxScene *)(nMaterial->getScene().userData));
}

////////////////////////////////////////////////////////////////////
//     Function : get_static_friction
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_static_friction() const {
  nassertr(nMaterial != NULL, -1.0f);

  return nMaterial->getStaticFriction();
}

////////////////////////////////////////////////////////////////////
//     Function : get_static_friction_v
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_static_friction_v() const {
  nassertr(nMaterial != NULL, -1.0f);

  return nMaterial->getStaticFrictionV();
}

////////////////////////////////////////////////////////////////////
//     Function : load_from_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
load_from_desc(const PhysxMaterialDesc & desc) {
  nassertv(nMaterial != NULL);

  nMaterial->loadFromDesc(desc.nMaterialDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : save_to_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
save_to_desc(PhysxMaterialDesc & desc) const {
  nassertv(nMaterial != NULL);

  nMaterial->saveToDesc(desc.nMaterialDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_dir_of_anisotropy
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_dir_of_anisotropy(const LVecBase3f & vec) {
  nassertv(nMaterial != NULL);

  nMaterial->setDirOfAnisotropy(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_dynamic_friction
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_dynamic_friction(float coef) {
  nassertv(nMaterial != NULL);

  nMaterial->setDynamicFriction(coef);
}

////////////////////////////////////////////////////////////////////
//     Function : set_dynamic_friction_v
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_dynamic_friction_v(float coef) {
  nassertv(nMaterial != NULL);

  nMaterial->setDynamicFrictionV(coef);
}

////////////////////////////////////////////////////////////////////
//     Function : set_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_flags(unsigned int flags) {
  nassertv(nMaterial != NULL);

  nMaterial->setFlags(flags);
}

////////////////////////////////////////////////////////////////////
//     Function : set_friction_combine_mode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_friction_combine_mode(PhysxCombineMode comb_mode) {
  nassertv(nMaterial != NULL);

  nMaterial->setFrictionCombineMode((NxCombineMode)comb_mode);
}

////////////////////////////////////////////////////////////////////
//     Function : set_restitution
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_restitution(float rest) {
  nassertv(nMaterial != NULL);

  nMaterial->setRestitution(rest);
}

////////////////////////////////////////////////////////////////////
//     Function : set_restitution_combine_mode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_restitution_combine_mode(PhysxCombineMode comb_mode) {
  nassertv(nMaterial != NULL);

  nMaterial->setRestitutionCombineMode((NxCombineMode)comb_mode);
}

////////////////////////////////////////////////////////////////////
//     Function : set_static_friction
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_static_friction(float coef) {
  nassertv(nMaterial != NULL);

  nMaterial->setStaticFriction(coef);
}

////////////////////////////////////////////////////////////////////
//     Function : set_static_friction_v
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_static_friction_v(float coef) {
  nassertv(nMaterial != NULL);

  nMaterial->setStaticFrictionV(coef);
}

#endif // HAVE_PHYSX

