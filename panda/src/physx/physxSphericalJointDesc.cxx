/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSphericalJointDesc.cxx
 * @author enn0x
 * @date 2009-09-28
 */

#include "physxSphericalJointDesc.h"
#include "physxManager.h"
#include "physxSpringDesc.h"
#include "physxJointLimitDesc.h"

/**
 * Set the distance above which to project joint.
 */
void PhysxSphericalJointDesc::
set_projection_distance(float distance) {

  _desc.projectionDistance = distance;
}

/**
 * Sets or clears a single SphericalJointFlag flag.
 */
void PhysxSphericalJointDesc::
set_flag(PhysxSphericalJointFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

/**
 * Sets a spring that works against twisting.
 */
void PhysxSphericalJointDesc::
set_twist_spring(const PhysxSpringDesc &spring) {

  _desc.twistSpring = spring._desc;
}

/**
 * Sets a spring that works against swinging.
 */
void PhysxSphericalJointDesc::
set_swing_spring(const PhysxSpringDesc &spring) {

  _desc.swingSpring = spring._desc;
}

/**
 * Sets a spring that lets the joint get pulled apart.
 */
void PhysxSphericalJointDesc::
set_joint_spring(const PhysxSpringDesc &spring) {

  _desc.jointSpring = spring._desc;
}

/**
 * Set the swing limit axis defined in the joint space of actor 0.
 */
void PhysxSphericalJointDesc::
set_swing_axis(const LVector3f &axis) {

  nassertv( !axis.is_nan() );
  _desc.swingAxis = PhysxManager::vec3_to_nxVec3(axis);
}

/**
 * Use this to enable joint projection.  Default is PM_none.
 */
void PhysxSphericalJointDesc::
set_projection_mode(PhysxProjectionMode mode) {

  _desc.projectionMode = (NxJointProjectionMode)mode;
}

/**
 * Limits rotation around twist axis.
 */
void PhysxSphericalJointDesc::
set_twist_limit_low(const PhysxJointLimitDesc &low) {

  _desc.twistLimit.low = low._desc;
}

/**
 * Limits rotation around twist axis.
 */
void PhysxSphericalJointDesc::
set_twist_limit_high(const PhysxJointLimitDesc &high) {

  _desc.twistLimit.high = high._desc;
}

/**
 * Limits swing of twist axis.
 */
void PhysxSphericalJointDesc::
set_swing_limit(const PhysxJointLimitDesc &limit) {

  _desc.swingLimit = limit._desc;
}

/**
 *
 */
float PhysxSphericalJointDesc::
get_projection_distance() const {

  return _desc.projectionDistance;
}

/**
 *
 */
bool PhysxSphericalJointDesc::
get_flag(PhysxSphericalJointFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

/**
 *
 */
PhysxSpringDesc PhysxSphericalJointDesc::
get_twist_spring() const {

  PhysxSpringDesc value;
  value._desc = _desc.twistSpring;
  return value;
}

/**
 *
 */
PhysxSpringDesc PhysxSphericalJointDesc::
get_swing_spring() const {

  PhysxSpringDesc value;
  value._desc = _desc.swingSpring;
  return value;
}

/**
 *
 */
PhysxSpringDesc PhysxSphericalJointDesc::
get_joint_spring() const {

  PhysxSpringDesc value;
  value._desc = _desc.jointSpring;
  return value;
}

/**
 *
 */
LVector3f PhysxSphericalJointDesc::
get_swing_axis() const {

  return PhysxManager::nxVec3_to_vec3(_desc.swingAxis);
}

/**
 *
 */
PhysxEnums::PhysxProjectionMode PhysxSphericalJointDesc::
get_projection_mode() const {

  return (PhysxProjectionMode)_desc.projectionMode;
}

/**
 *
 */
PhysxJointLimitDesc PhysxSphericalJointDesc::
get_twist_limit_low() const {

  PhysxJointLimitDesc value;
  value._desc = _desc.twistLimit.low;
  return value;
}

/**
 *
 */
PhysxJointLimitDesc PhysxSphericalJointDesc::
get_twist_limit_high() const {

  PhysxJointLimitDesc value;
  value._desc = _desc.twistLimit.high;
  return value;
}

/**
 * Limits swing of twist axis.
 */
PhysxJointLimitDesc PhysxSphericalJointDesc::
get_swing_limit() const {

  PhysxJointLimitDesc value;
  value._desc = _desc.swingLimit;
  return value;
}
