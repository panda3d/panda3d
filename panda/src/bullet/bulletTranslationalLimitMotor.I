/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTranslationalLimitMotor.I
 * @author enn0x
 * @date 2013-03-03
 */

/**
 *
 */
INLINE bool BulletTranslationalLimitMotor::
is_limited(int axis) const {

  nassertr((0 <= axis) && (axis <= 2), false);
  return _motor.isLimited(axis);
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_motor_enabled(int axis, bool enabled) {

  nassertv((0 <= axis) && (axis <= 2));
  _motor.m_enableMotor[axis] = enabled;
}

/**
 *
 */
INLINE bool BulletTranslationalLimitMotor::
get_motor_enabled(int axis) const {

  nassertr((0 <= axis) && (axis <= 2), false);
  return _motor.m_enableMotor[axis];
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_low_limit(const LVecBase3 &limit) {

  nassertv(!limit.is_nan());
  _motor.m_lowerLimit = LVecBase3_to_btVector3(limit);
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_high_limit(const LVecBase3 &limit) {

  nassertv(!limit.is_nan());
  _motor.m_upperLimit = LVecBase3_to_btVector3(limit);
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_target_velocity(const LVecBase3 &velocity) {

  nassertv(!velocity.is_nan());
  _motor.m_targetVelocity = LVecBase3_to_btVector3(velocity);
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_max_motor_force(const LVecBase3 &force) {

  nassertv(!force.is_nan());
  _motor.m_maxMotorForce = LVecBase3_to_btVector3(force);
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_damping(PN_stdfloat damping) {

  _motor.m_damping = (btScalar)damping;
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_softness(PN_stdfloat softness) {

  _motor.m_limitSoftness = (btScalar)softness;
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_restitution(PN_stdfloat restitution) {

  _motor.m_restitution = (btScalar)restitution;
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_normal_cfm(const LVecBase3 &cfm) {

  nassertv(!cfm.is_nan());
  _motor.m_normalCFM = LVecBase3_to_btVector3(cfm);
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_stop_cfm(const LVecBase3 &cfm) {

  nassertv(!cfm.is_nan());
  _motor.m_stopCFM = LVecBase3_to_btVector3(cfm);
}

/**
 *
 */
INLINE void BulletTranslationalLimitMotor::
set_stop_erp(const LVecBase3 &erp) {

  nassertv(!erp.is_nan());
  _motor.m_stopERP = LVecBase3_to_btVector3(erp);
}

/**
 * Retrieves the current value of angle: 0 = free, 1 = at low limit, 2 = at
 * high limit.
 */
INLINE int BulletTranslationalLimitMotor::
get_current_limit(int axis) const {

  nassertr((0 <= axis) && (axis <= 2), false);
  return _motor.m_currentLimit[axis];
}

/**
 *
 */
INLINE LVector3 BulletTranslationalLimitMotor::
get_current_error() const {

  return btVector3_to_LVector3(_motor.m_currentLimitError);
}

/**
 *
 */
INLINE LPoint3 BulletTranslationalLimitMotor::
get_current_diff() const {

  return btVector3_to_LPoint3(_motor.m_currentLinearDiff);
}

/**
 *
 */
INLINE LVector3 BulletTranslationalLimitMotor::
get_accumulated_impulse() const {

  return btVector3_to_LVector3(_motor.m_accumulatedImpulse);
}
