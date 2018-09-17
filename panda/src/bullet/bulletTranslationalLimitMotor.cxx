/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTranslationalLimitMotor.cxx
 * @author enn0x
 * @date 2013-03-03
 */

#include "bulletTranslationalLimitMotor.h"

#include "bulletWorld.h"

/**
 *
 */
BulletTranslationalLimitMotor::
BulletTranslationalLimitMotor(btTranslationalLimitMotor &motor)
 : _motor(motor) {

}

/**
 *
 */
BulletTranslationalLimitMotor::
BulletTranslationalLimitMotor(const BulletTranslationalLimitMotor &copy)
  : _motor(copy._motor) {

}

/**
 *
 */
bool BulletTranslationalLimitMotor::
is_limited(int axis) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr((0 <= axis) && (axis <= 2), false);
  return _motor.isLimited(axis);
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_motor_enabled(int axis, bool enabled) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv((0 <= axis) && (axis <= 2));
  _motor.m_enableMotor[axis] = enabled;
}

/**
 *
 */
bool BulletTranslationalLimitMotor::
get_motor_enabled(int axis) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr((0 <= axis) && (axis <= 2), false);
  return _motor.m_enableMotor[axis];
}


/**
 *
 */
void BulletTranslationalLimitMotor::
set_low_limit(const LVecBase3 &limit) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!limit.is_nan());
  _motor.m_lowerLimit = LVecBase3_to_btVector3(limit);
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_high_limit(const LVecBase3 &limit) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!limit.is_nan());
  _motor.m_upperLimit = LVecBase3_to_btVector3(limit);
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_target_velocity(const LVecBase3 &velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!velocity.is_nan());
  _motor.m_targetVelocity = LVecBase3_to_btVector3(velocity);
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_max_motor_force(const LVecBase3 &force) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!force.is_nan());
  _motor.m_maxMotorForce = LVecBase3_to_btVector3(force);
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_damping(PN_stdfloat damping) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_damping = (btScalar)damping;
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_softness(PN_stdfloat softness) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_limitSoftness = (btScalar)softness;
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_restitution(PN_stdfloat restitution) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_restitution = (btScalar)restitution;
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_normal_cfm(const LVecBase3 &cfm) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!cfm.is_nan());
  _motor.m_normalCFM = LVecBase3_to_btVector3(cfm);
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_stop_cfm(const LVecBase3 &cfm) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!cfm.is_nan());
  _motor.m_stopCFM = LVecBase3_to_btVector3(cfm);
}

/**
 *
 */
void BulletTranslationalLimitMotor::
set_stop_erp(const LVecBase3 &erp) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!erp.is_nan());
  _motor.m_stopERP = LVecBase3_to_btVector3(erp);
}

/**
 * Retrieves the current value of angle: 0 = free, 1 = at low limit, 2 = at
 * high limit.
 */
int BulletTranslationalLimitMotor::
get_current_limit(int axis) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr((0 <= axis) && (axis <= 2), false);
  return _motor.m_currentLimit[axis];
}

/**
 *
 */
LVector3 BulletTranslationalLimitMotor::
get_current_error() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_motor.m_currentLimitError);
}

/**
 *
 */
LPoint3 BulletTranslationalLimitMotor::
get_current_diff() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_motor.m_currentLinearDiff);
}

/**
 *
 */
LVector3 BulletTranslationalLimitMotor::
get_accumulated_impulse() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_motor.m_accumulatedImpulse);
}
