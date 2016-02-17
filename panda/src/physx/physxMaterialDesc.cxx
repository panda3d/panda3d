/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMaterialDesc.cxx
 * @author enn0x
 * @date 2009-09-21
 */

#include "physxMaterialDesc.h"
#include "physxManager.h"

/**
 * Sets the coefficient of restitution -- 0 makes the object bounce as little as
 * possible, higher values up to 1.0 result in more bounce.  Note that values
 * close to or above 1 may cause stability problems and/or increasing energy.
 * Range: [0,1] Default: 0.0
 */
void PhysxMaterialDesc::
set_restitution(float restitution) {

  _desc.restitution = restitution;
}

/**
 * Sets the coefficient of static friction -- should be in [0, +inf]. If the
 * flag MF_anisotropic is set, then this value is used for the primary direction
 * of anisotropy (U axis).
 */
void PhysxMaterialDesc::
set_static_friction(float coef) {

  _desc.staticFriction = coef;
}

/**
 * Sets the coefficient of dynamic friction -- should be in [0, +inf]. If set to
 * greater than staticFriction, the effective value of staticFriction will be
 * increased to match.  If the flag MF_anisotropic is set, then this value is
 * used for the primary direction of anisotropy (U axis).
 */
void PhysxMaterialDesc::
set_dynamic_friction(float coef) {

  _desc.dynamicFriction = coef;
}

/**
 * Sets the anisotropic static friction coefficient for along the secondary (V)
 * axis of anisotropy.  This is only used if the flag MF_anisotropic is set.
 */
void PhysxMaterialDesc::
set_static_friction_v(float coef) {

  _desc.staticFrictionV = coef;
}

/**
 * Sets the anisotropic dynamic friction coefficient for along the secondary (V)
 * axis of anisotropy.  This is only used if the flag MF_anisotropic is set.
 */
void PhysxMaterialDesc::
set_dynamic_friction_v(float coef) {

  _desc.dynamicFrictionV = coef;
}

/**
 * Sets flags which control the behavior of a material.
 */
void PhysxMaterialDesc::
set_flag(PhysxMaterialFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

/**
 * Sets the shape space direction (unit vector) of anisotropy.  This is only
 * used if the flag MF_anisotropic is set.
 */
void PhysxMaterialDesc::
set_dir_of_anisotropy(const LVector3f dir) {

  _desc.dirOfAnisotropy = PhysxManager::vec3_to_nxVec3(dir);
}

/**
 * Sets the friction combine mode.  - CM_average : Average: (a + b)/2. - CM_min
 * : Minimum: min(a,b). - CM_multiply : Multiply: a*b.  - CM_max : Maximum:
 * max(a,b).
 */
void PhysxMaterialDesc::
set_friction_combine_mode(PhysxCombineMode mode) {

  _desc.frictionCombineMode = (NxCombineMode)mode;
}

/**
 * Sets the restitution combine mode.  - CM_average : Average: (a + b)/2. -
 * CM_min : Minimum: min(a,b). - CM_multiply : Multiply: a*b.  - CM_max :
 * Maximum: max(a,b).
 */
void PhysxMaterialDesc::
set_restitution_combine_mode(PhysxCombineMode mode) {

  _desc.restitutionCombineMode = (NxCombineMode)mode;
}

/**
 * Returns the coefficient of restitution.
 */
float PhysxMaterialDesc::
get_restitution() const {

  return _desc.restitution;
}

/**
 * Retruns the coefficient of static friction.
 */
float PhysxMaterialDesc::
get_static_friction() const {

  return _desc.staticFriction;
}

/**
 * Returns the coefficient of dynamic friction.
 */
float PhysxMaterialDesc::
get_dynamic_friction() const {

  return _desc.dynamicFriction;
}

/**
 * Returns the anisotropic static friction coefficient for along the secondary
 * (V) axis of anisotropy.
 */
float PhysxMaterialDesc::
get_static_friction_v() const {

  return _desc.staticFrictionV;
}

/**
 * Returns the anisotropic dynamic friction coefficient for along the secondary
 * (V) axis of anisotropy.
 */
float PhysxMaterialDesc::
get_dynamic_friction_v() const {

  return _desc.dynamicFrictionV;
}

/**
 * Returns flags which control the behavior of a material.
 */
bool PhysxMaterialDesc::
get_flag(PhysxMaterialFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

/**
 * Returns the shape space direction (unit vector) of anisotropy.
 */
LVector3f PhysxMaterialDesc::
get_dir_of_anisotropy() const {

  return PhysxManager::nxVec3_to_vec3(_desc.dirOfAnisotropy);
}

/**
 * Returns the friction combine mode.
 */
PhysxEnums::PhysxCombineMode PhysxMaterialDesc::
get_friction_combine_mode() const {

  return (PhysxCombineMode)_desc.frictionCombineMode;
}

/**
 * Returns the restitution combine mode.
 */
PhysxEnums::PhysxCombineMode PhysxMaterialDesc::
get_restitution_combine_mode() const {

  return (PhysxCombineMode)_desc.restitutionCombineMode;
}
