// Filename: physxMaterial.cxx
// Created by:  enn0x (21Sep09)
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

#include "physxMaterial.h"
#include "physxMaterialDesc.h"
#include "physxManager.h"

TypeHandle PhysxMaterial::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
link(NxMaterial *materialPtr) {

  // Link self
  _ptr = materialPtr;
  _ptr->userData = this;
  _error_type = ET_ok;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_materials.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
unlink() {

  // Unlink self
  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_materials.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::release
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  _ptr->getScene().releaseMaterial(*_ptr);
  _ptr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_scene
//       Access: Published
//  Description: Returns the scene that owns this material.
////////////////////////////////////////////////////////////////////
PhysxScene *PhysxMaterial::
get_scene() const {

  nassertr(_error_type == ET_ok, NULL);
  return (PhysxScene *)(_ptr->getScene().userData);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_material_index
//       Access: Published
//  Description: Returns the material index for this material.
//
//               Materials are associated with mesh faces and shapes
//               using material index identifiers.
//
//               If you release a material while its material index
//               is still in use by shapes or meshes, the material
//               usage of these objects becomes undefined as the
//               material index gets recycled.
////////////////////////////////////////////////////////////////////
unsigned short PhysxMaterial::
get_material_index() const {

  nassertr(_error_type == ET_ok, 0);
  return _ptr->getMaterialIndex();
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxMaterial::load_from_desc
//       Access : Published
//  Description : Loads the entire state of the material from a 
//                descriptor with a single call.
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
load_from_desc(const PhysxMaterialDesc &materialDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(materialDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxMaterial::save_to_desc
//       Access : Published
//  Description : Saves the state of the material object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
save_to_desc(PhysxMaterialDesc & materialDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(materialDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_restitution
//       Access: Published
//  Description: Sets the coefficient of restitution. 
//               A coefficient of 0 makes the object bounce as
//               little as possible, higher values up to 1.0 result
//               in more bounce.
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_restitution(float restitution) {

  nassertv(_error_type == ET_ok);
  _ptr->setRestitution(restitution);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_restitution
//       Access: Published
//  Description: Returns the coefficient of restitution.
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_restitution() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRestitution();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_static_friction
//       Access: Published
//  Description: Sets the coefficient of static friction. 
//               The coefficient of static friction should be in the
//               range [0, +inf].
//               If the flag MF_anisotropic is set, then this value
//               is used for the primary direction of anisotropy
//               (U axis).
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_static_friction(float coef) {

  nassertv(_error_type == ET_ok);
  _ptr->setStaticFriction(coef);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_static_friction
//       Access: Published
//  Description: Returns the coefficient of static friction.
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_static_friction() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getStaticFriction();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_dynamic_friction
//       Access: Published
//  Description: Sets the coefficient of dynamic friction. 
//               The coefficient of dynamic friction should be in
//               [0, +inf]. If set to greater than staticFriction,
//               the effective value of staticFriction will be
//               increased to match.
//               If the flag MF_anisotropic is set, then this value
//               is used for the primary direction of anisotropy
//               (U axis).
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_dynamic_friction(float coef) {

  nassertv(_error_type == ET_ok);
  _ptr->setDynamicFriction(coef);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_dynamic_friction
//       Access: Published
//  Description: Returns the DynamicFriction value.
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_dynamic_friction() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getDynamicFriction();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_static_friction_v
//       Access: Published
//  Description: Sets the static friction coefficient along the
//               secondary (V) axis. This is used when anisotropic
//               friction is being applied. I.e. the flag
//               MF_anisotropic is set.
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_static_friction_v(float coef) {

  nassertv(_error_type == ET_ok);
  _ptr->setStaticFrictionV(coef);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_static_friction_v
//       Access: Published
//  Description: Returns the static friction coefficient for the
//               V direction.
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_static_friction_v() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getStaticFrictionV();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_dynamic_friction_v
//       Access: Published
//  Description: Sets the dynamic friction coefficient along the
//               secondary (V) axis. This is used when anisotropic
//               friction is being applied. I.e. the flag
//               MF_anisotropic is set.
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_dynamic_friction_v(float coef) {

  nassertv(_error_type == ET_ok);
  _ptr->setDynamicFrictionV(coef);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_dynamic_friction_v
//       Access: Published
//  Description: Returns the dynamic friction coefficient for the
//               V direction.
////////////////////////////////////////////////////////////////////
float PhysxMaterial::
get_dynamic_friction_v() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getDynamicFrictionV();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_flag
//       Access: Published
//  Description: Sets the value of a single flag.
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_flag(PhysxMaterialFlag flag, bool value) {

  nassertv(_error_type == ET_ok);
  NxU32 flags = _ptr->getFlags();

  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }

  _ptr->setFlags(flags);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_flag
//       Access: Published
//  Description: Returns the value of a single flag.
////////////////////////////////////////////////////////////////////
bool PhysxMaterial::
get_flag(PhysxMaterialFlag flag) const {

  nassertr(_error_type == ET_ok, false);
  return (_ptr->getFlags() & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_dir_of_anisotropy
//       Access: Published
//  Description: Sets the shape space direction (unit vector) of
//               anisotropy. This is only used if the flag 
//               MF_anisotropic is set.
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_dir_of_anisotropy(const LVector3f dir) {

  nassertv(_error_type == ET_ok);
  _ptr->setDirOfAnisotropy(PhysxManager::vec3_to_nxVec3(dir));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_dir_of_anisotropy
//       Access: Published
//  Description: Returns the direction of anisotropy value.
////////////////////////////////////////////////////////////////////
LVector3f PhysxMaterial::
get_dir_of_anisotropy() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getDirOfAnisotropy());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_friction_combine_mode
//       Access: Published
//  Description: Sets the friction combine mode.
//               - CM_average : Average: (a + b)/2.
//               - CM_min : Minimum: min(a,b).
//               - CM_multiply : Multiply: a*b.
//               - CM_max : Maximum: max(a,b).
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_friction_combine_mode(PhysxCombineMode mode) {

  nassertv(_error_type == ET_ok);
  _ptr->setFrictionCombineMode((NxCombineMode)mode);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_friction_combine_mode
//       Access: Published
//  Description: Returns the friction combine mode.
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxCombineMode PhysxMaterial::
get_friction_combine_mode() const {

  nassertr(_error_type == ET_ok, CM_average);
  return (PhysxCombineMode)_ptr->getFrictionCombineMode();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::set_restitution_combine_mode
//       Access: Published
//  Description: Sets the restitution combine mode.
//               - CM_average : Average: (a + b)/2.
//               - CM_min : Minimum: min(a,b).
//               - CM_multiply : Multiply: a*b.
//               - CM_max : Maximum: max(a,b).
////////////////////////////////////////////////////////////////////
void PhysxMaterial::
set_restitution_combine_mode(PhysxCombineMode mode) {

  nassertv(_error_type == ET_ok);
  _ptr->setRestitutionCombineMode((NxCombineMode)mode);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterial::get_restitution_combine_mode
//       Access: Published
//  Description: Returns the restitution combine mode.
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxCombineMode PhysxMaterial::
get_restitution_combine_mode() const {

  nassertr(_error_type == ET_ok, CM_average);
  return (PhysxCombineMode)_ptr->getRestitutionCombineMode();
}

