// Filename: physxMaterialDesc.cxx
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

#include "physxMaterialDesc.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_restitution
//       Access: Published
//  Description: Sets the coefficient of restitution -- 0 makes the
//               object bounce as little as possible, higher values
//               up to 1.0 result in more bounce. Note that values
//               close to or above 1 may cause stability problems
//               and/or increasing energy. Range: [0,1]
//               Default: 0.0
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_restitution(float restitution) {

  _desc.restitution = restitution;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_static_friction
//       Access: Published
//  Description: Sets the coefficient of static friction -- should
//               be in [0, +inf].
//               If the flag MF_anisotropic is set, then this value
//               is used for the primary direction of anisotropy
//               (U axis).
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_static_friction(float coef) {

  _desc.staticFriction = coef;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_dynamic_friction
//       Access: Published
//  Description: Sets the coefficient of dynamic friction -- should
//               be in [0, +inf]. If set to greater than
//               staticFriction, the effective value of staticFriction
//               will be increased to match.
//               If the flag MF_anisotropic is set, then this value
//               is used for the primary direction of anisotropy
//               (U axis).
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_dynamic_friction(float coef) {

  _desc.dynamicFriction = coef;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_static_friction_v
//       Access: Published
//  Description: Sets the anisotropic static friction coefficient
//               for along the secondary (V) axis of anisotropy.
//               This is only used if the flag MF_anisotropic is
//               set.
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_static_friction_v(float coef) {

  _desc.staticFrictionV = coef;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_dynamic_friction_v
//       Access: Published
//  Description: Sets the anisotropic dynamic friction coefficient
//               for along the secondary (V) axis of anisotropy.
//               This is only used if the flag MF_anisotropic is
//               set.
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_dynamic_friction_v(float coef) {

  _desc.dynamicFrictionV = coef;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_flag
//       Access: Published
//  Description: Sets flags which control the behavior of a
//               material.
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_flag(PhysxMaterialFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_dir_of_anisotropy
//       Access: Published
//  Description: Sets the shape space direction (unit vector) of
//               anisotropy.
//               This is only used if the flag MF_anisotropic is
//               set.
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_dir_of_anisotropy(const LVector3f dir) {

  _desc.dirOfAnisotropy = PhysxManager::vec3_to_nxVec3(dir);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_friction_combine_mode
//       Access: Published
//  Description: Sets the friction combine mode.
//               - CM_average : Average: (a + b)/2.
//               - CM_min : Minimum: min(a,b).
//               - CM_multiply : Multiply: a*b.
//               - CM_max : Maximum: max(a,b).
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_friction_combine_mode(PhysxCombineMode mode) {

  _desc.frictionCombineMode = (NxCombineMode)mode;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::set_restitution_combine_mode
//       Access: Published
//  Description: Sets the restitution combine mode.
//               - CM_average : Average: (a + b)/2.
//               - CM_min : Minimum: min(a,b).
//               - CM_multiply : Multiply: a*b.
//               - CM_max : Maximum: max(a,b).
////////////////////////////////////////////////////////////////////
void PhysxMaterialDesc::
set_restitution_combine_mode(PhysxCombineMode mode) {

  _desc.restitutionCombineMode = (NxCombineMode)mode;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_restitution
//       Access: Published
//  Description: Returns the coefficient of restitution.
////////////////////////////////////////////////////////////////////
float PhysxMaterialDesc::
get_restitution() const {

  return _desc.restitution;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_static_friction
//       Access: Published
//  Description: Retruns the coefficient of static friction.
////////////////////////////////////////////////////////////////////
float PhysxMaterialDesc::
get_static_friction() const {

  return _desc.staticFriction;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_dynamic_friction
//       Access: Published
//  Description: Returns the coefficient of dynamic friction.
////////////////////////////////////////////////////////////////////
float PhysxMaterialDesc::
get_dynamic_friction() const {

  return _desc.dynamicFriction;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_static_friction_v
//       Access: Published
//  Description: Returns the anisotropic static friction coefficient
//               for along the secondary (V) axis of anisotropy.
////////////////////////////////////////////////////////////////////
float PhysxMaterialDesc::
get_static_friction_v() const {

  return _desc.staticFrictionV;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_dynamic_friction_v
//       Access: Published
//  Description: Returns the anisotropic dynamic friction
//               coefficient for along the secondary (V) axis of
//               anisotropy.
////////////////////////////////////////////////////////////////////
float PhysxMaterialDesc::
get_dynamic_friction_v() const {

  return _desc.dynamicFrictionV;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_flag
//       Access: Published
//  Description: Returns flags which control the behavior of a
//               material.
////////////////////////////////////////////////////////////////////
bool PhysxMaterialDesc::
get_flag(PhysxMaterialFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_dir_of_anisotropy
//       Access: Published
//  Description: Returns the shape space direction (unit vector) of
//               anisotropy.
////////////////////////////////////////////////////////////////////
LVector3f PhysxMaterialDesc::
get_dir_of_anisotropy() const {

  return PhysxManager::nxVec3_to_vec3(_desc.dirOfAnisotropy);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_friction_combine_mode
//       Access: Published
//  Description: Returns the friction combine mode.
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxCombineMode PhysxMaterialDesc::
get_friction_combine_mode() const {

  return (PhysxCombineMode)_desc.frictionCombineMode;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxMaterialDesc::get_restitution_combine_mode
//       Access: Published
//  Description: Returns the restitution combine mode.
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxCombineMode PhysxMaterialDesc::
get_restitution_combine_mode() const {

  return (PhysxCombineMode)_desc.restitutionCombineMode;
}

