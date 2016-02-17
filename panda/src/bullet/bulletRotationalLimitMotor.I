/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletRotationalLimitMotor.I
 * @author enn0x
 * @date 2013-03-03
 */

/**
 *
 */
INLINE bool BulletRotationalLimitMotor::
is_limited() const {

  return _motor.isLimited();
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_motor_enabled(bool enabled) {

  _motor.m_enableMotor = enabled;
}

/**
 *
 */
INLINE bool BulletRotationalLimitMotor::
get_motor_enabled() const {

  return _motor.m_enableMotor;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_low_limit(PN_stdfloat limit) {

  _motor.m_loLimit = (btScalar)limit;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_high_limit(PN_stdfloat limit) {

  _motor.m_hiLimit = (btScalar)limit;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_target_velocity(PN_stdfloat velocity) {

  _motor.m_targetVelocity = (btScalar)velocity;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_max_motor_force(PN_stdfloat force) {

  _motor.m_maxMotorForce = (btScalar)force;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_max_limit_force(PN_stdfloat force) {

  _motor.m_maxLimitForce = (btScalar)force;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_damping(PN_stdfloat damping) {

  _motor.m_damping = (btScalar)damping;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_softness(PN_stdfloat softness) {

  _motor.m_limitSoftness = (btScalar)softness;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_bounce(PN_stdfloat bounce) {

  _motor.m_bounce = (btScalar)bounce;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_normal_cfm(PN_stdfloat cfm) {

  _motor.m_normalCFM = (btScalar)cfm;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_stop_cfm(PN_stdfloat cfm) {

  _motor.m_stopCFM = (btScalar)cfm;
}

/**
 *
 */
INLINE void BulletRotationalLimitMotor::
set_stop_erp(PN_stdfloat erp) {

  _motor.m_stopERP = (btScalar)erp;
}

/**
 * Retrieves the current value of angle: 0 = free, 1 = at low limit, 2 = at
 * high limit.
 */
INLINE int BulletRotationalLimitMotor::
get_current_limit() const {

  return _motor.m_currentLimit;
}

/**
 *
 */
INLINE PN_stdfloat BulletRotationalLimitMotor::
get_current_error() const {

  return (PN_stdfloat)_motor.m_currentLimitError;
}

/**
 *
 */
INLINE PN_stdfloat BulletRotationalLimitMotor::
get_current_position() const {

  return (PN_stdfloat)_motor.m_currentPosition;
}

/**
 *
 */
INLINE PN_stdfloat BulletRotationalLimitMotor::
get_accumulated_impulse() const {

  return (PN_stdfloat)_motor.m_accumulatedImpulse;
}
