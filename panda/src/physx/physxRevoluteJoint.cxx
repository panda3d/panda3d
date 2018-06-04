/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxRevoluteJoint.cxx
 * @author enn0x
 * @date 2009-10-02
 */

#include "physxRevoluteJoint.h"
#include "physxRevoluteJointDesc.h"
#include "physxJointLimitDesc.h"
#include "physxMotorDesc.h"
#include "physxSpringDesc.h"

TypeHandle PhysxRevoluteJoint::_type_handle;

/**
 *
 */
void PhysxRevoluteJoint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isRevoluteJoint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

/**
 *
 */
void PhysxRevoluteJoint::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

/**
 * Saves the state of the joint object to a descriptor.
 */
void PhysxRevoluteJoint::
save_to_desc(PhysxRevoluteJointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

/**
 * Loads the entire state of the joint from a descriptor with a single call.
 */
void PhysxRevoluteJoint::
load_from_desc(const PhysxRevoluteJointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}

/**
 * Retrieves the current revolute joint angle.
 *
 * The relative orientation of the bodies is stored when the joint is created,
 * or when set_axis() or set_anchor() is called.  This initial orientation
 * returns an angle of zero, and joint angles are measured relative to this
 * pose.  The angle is in the range [-180, 180], with positive angles CCW
 * around the axis, measured from body2 to body1.
 */
float PhysxRevoluteJoint::
get_angle() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return NxMath::radToDeg(_ptr->getAngle());
}

/**
 * Retrieves the revolute joint angle's rate of change (angular velocity). It
 * is the angular velocity of body1 minus body2 projected along the axis.
 */
float PhysxRevoluteJoint::
get_velocity() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getVelocity();
}

/**
 * Sets the joint projection mode.
 */
void PhysxRevoluteJoint::
set_projection_mode(PhysxProjectionMode mode) {

  nassertv(_error_type == ET_ok);
  _ptr->setProjectionMode((NxJointProjectionMode)mode);
}

/**
 * Retrieves the joints projection mode.
 */
PhysxEnums::PhysxProjectionMode PhysxRevoluteJoint::
get_projection_mode() const {

  nassertr(_error_type == ET_ok, PM_none);
  return (PhysxProjectionMode)_ptr->getProjectionMode();
}

/**
 * Sets or clears a single RevoluteJointFlag.
 */
void PhysxRevoluteJoint::
set_flag(PhysxRevoluteJointFlag flag, bool value) {

  nassertv( _error_type == ET_ok );
  NxU32 flags = _ptr->getFlags();

  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }

  _ptr->setFlags(flags);
}

/**
 * Returns the value of a single RevoluteJointFlag.
 */
bool PhysxRevoluteJoint::
get_flag(PhysxRevoluteJointFlag flag) const {

  nassertr(_error_type == ET_ok, false);
  return (_ptr->getFlags() & flag) ? true : false;
}

/**
 * Sets spring parameters.
 *
 * The spring is implicitly integrated so no instability should result for
 * arbitrary spring and damping constants.  Using these settings together with
 * a motor is not possible -- the motor will have priority and the spring
 * settings are ignored.  If you would like to simulate your motor's internal
 * friction, do this by altering the motor parameters directly.
 *
 * spring - The rotational spring acts along the hinge axis and tries to force
 * the joint angle to zero.  A setting of zero disables the spring.  Default
 * is 0, should be >= 0.
 *
 * damper - Damping coefficient; acts against the hinge's angular velocity.  A
 * setting of zero disables the damping.  The default is 0, should be >= 0.
 *
 * targetValue - The angle at which the spring is relaxed.  In [-Pi,Pi].
 * Default is 0.
 *
 * This automatically enables the spring
 */
void PhysxRevoluteJoint::
set_spring(const PhysxSpringDesc &spring) {

  nassertv(_error_type == ET_ok);
  _ptr->setSpring(spring._desc);
}

/**
 * Sets motor parameters for the joint.
 *
 * For a positive velTarget, the motor pulls the first body towards its
 * pulley, for a negative velTarget, the motor pulls the second body towards
 * its pulley.
 *
 * velTarget - the relative velocity the motor is trying to achieve.  The
 * motor will only be able to reach this velocity if the maxForce is
 * sufficiently large.  If the joint is moving faster than this velocity, the
 * motor will actually try to brake.  If you set this to infinity then the
 * motor will keep speeding up, unless there is some sort of resistance on the
 * attached bodies.
 *
 * maxForce - the maximum force the motor can exert.  Zero disables the motor.
 * Default is 0, should be >= 0. Setting this to a very large value if
 * velTarget is also very large may not be a good idea.
 *
 * freeSpin - if this flag is set, and if the joint is moving faster than
 * velTarget, then neither braking nor additional acceleration will result.
 * default: false.
 *
 * This automatically enables the motor.
 */
void PhysxRevoluteJoint::
set_motor(const PhysxMotorDesc &motor) {

  nassertv(_error_type == ET_ok);
  _ptr->setMotor(motor._desc);
}

/**
 * Sets angular joint limits.
 *
 * If either of these limits are set, any planar limits in PhysxJoint are
 * ignored.  The limits are angles defined the same way as the values that
 * get_angle() returns.
 *
 * The following has to hold:
 *
 * Pi < lowAngle < highAngle < Pi Both limits are disabled by default.  Also
 * sets coefficients of restitutions for the low and high angular limits.
 * These settings are only used if valid limits are set using set_limits().
 * These restitution coefficients work the same way as for contacts.
 *
 * The coefficient of restitution determines whether a collision with the
 * joint limit is completely elastic (like pool balls, restitution = 1, no
 * energy is lost in the collision), completely inelastic (like putty,
 * restitution = 0, no rebound after collision) or somewhere in between.  The
 * default is 0 for both.
 *
 * This automatically enables the limit.
 */
void PhysxRevoluteJoint::
set_limits(const PhysxJointLimitDesc &low, const PhysxJointLimitDesc &high) {

  nassertv(_error_type == ET_ok);

  NxJointLimitPairDesc limits;
  limits.low = low._desc;
  limits.high = high._desc;
  _ptr->setLimits(limits);
}

/**
 *
 */
PhysxMotorDesc PhysxRevoluteJoint::
get_motor() const {

  nassertr(_error_type == ET_ok, PhysxMotorDesc(0));

  PhysxMotorDesc value;
  _ptr->getMotor(value._desc);
  return value;
}

/**
 *
 */
PhysxSpringDesc PhysxRevoluteJoint::
get_spring() const {

  nassertr(_error_type == ET_ok, PhysxSpringDesc(0));

  PhysxSpringDesc value;
  _ptr->getSpring(value._desc);
  return value;
}
