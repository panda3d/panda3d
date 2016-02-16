/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxRevoluteJointDesc.cxx
 * @author enn0x
 * @date 2009-09-28
 */

#include "physxRevoluteJointDesc.h"
#include "physxSpringDesc.h"
#include "physxMotorDesc.h"
#include "physxJointLimitDesc.h"

/**
 * Sets the distance beyond which the joint is projected.
 */
void PhysxRevoluteJointDesc::
set_projection_distance(float distance) {

  _desc.projectionDistance = distance;
}

/**
 * Sets the angle beyond which the joint is projected.
 */
void PhysxRevoluteJointDesc::
set_projection_angle(float angle) {

  _desc.projectionAngle = angle;
}

/**
 * Sets an aptional spring.
 */
void PhysxRevoluteJointDesc::
set_spring(const PhysxSpringDesc &spring) {

  _desc.spring = spring._desc;
}

/**
 * Sets or clears a single RevoluteJointFlag flag.
 */
void PhysxRevoluteJointDesc::
set_flag(PhysxRevoluteJointFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

/**
 * Use this to enable joint projection.  Default is PM_none.
 */
void PhysxRevoluteJointDesc::
set_projection_mode(PhysxProjectionMode mode) {

  _desc.projectionMode = (NxJointProjectionMode)mode;
}

/**
 * Sets an optional joint motor.
 */
void PhysxRevoluteJointDesc::
set_motor(const PhysxMotorDesc &motor) {

  _desc.motor = motor._desc;
}

/**
 * Sets optional limits for the angular motion of the joint.
 */
void PhysxRevoluteJointDesc::
set_limit_low(const PhysxJointLimitDesc &low) {

  _desc.limit.low = low._desc;
}

/**
 * Sets optional limits for the angular motion of the joint.
 */
void PhysxRevoluteJointDesc::
set_limit_high(const PhysxJointLimitDesc &high) {

  _desc.limit.high = high._desc;
}

/**
 * Return the distance beyond which the joint is projected.
 */
float PhysxRevoluteJointDesc::
get_projection_distance() const {

  return _desc.projectionDistance;
}

/**
 * Return the angle beyond which the joint is projected.
 */
float PhysxRevoluteJointDesc::
get_projection_angle() const {

  return _desc.projectionAngle;
}

/**

 */
PhysxSpringDesc PhysxRevoluteJointDesc::
get_spring() const {

  PhysxSpringDesc value;
  value._desc = _desc.spring;
  return value;
}

/**

 */
bool PhysxRevoluteJointDesc::
get_flag(PhysxRevoluteJointFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

/**

 */
PhysxEnums::PhysxProjectionMode PhysxRevoluteJointDesc::
get_projection_mode() const {

  return (PhysxProjectionMode)_desc.projectionMode;
}

/**
 * Sets an optional joint motor.
 */
PhysxMotorDesc PhysxRevoluteJointDesc::
get_motor() const {

  PhysxMotorDesc value;
  value._desc = _desc.motor;
  return value;
}

/**

 */
PhysxJointLimitDesc PhysxRevoluteJointDesc::
get_limit_low() const {

  PhysxJointLimitDesc value;
  value._desc = _desc.limit.low;
  return value;
}

/**

 */
PhysxJointLimitDesc PhysxRevoluteJointDesc::
get_limit_high() const {

  PhysxJointLimitDesc value;
  value._desc = _desc.limit.high;
  return value;
}
