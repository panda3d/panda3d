/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletRotationalLimitMotor.cxx
 * @author enn0x
 * @date 2013-03-03
 */

#include "bulletRotationalLimitMotor.h"

#include "bulletWorld.h"

/**
 *
 */
BulletRotationalLimitMotor::
BulletRotationalLimitMotor(btRotationalLimitMotor &motor)
 : _motor(motor) {

}

/**
 *
 */
BulletRotationalLimitMotor::
BulletRotationalLimitMotor(const BulletRotationalLimitMotor &copy)
  : _motor(copy._motor) {

}

/**
 *
 */
bool BulletRotationalLimitMotor::
is_limited() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _motor.isLimited();
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_motor_enabled(bool enabled) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_enableMotor = enabled;
}

/**
 *
 */
bool BulletRotationalLimitMotor::
get_motor_enabled() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _motor.m_enableMotor;
}
/**
 *
 */
void BulletRotationalLimitMotor::
set_low_limit(PN_stdfloat limit) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_loLimit = (btScalar)limit;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_high_limit(PN_stdfloat limit) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_hiLimit = (btScalar)limit;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_target_velocity(PN_stdfloat velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_targetVelocity = (btScalar)velocity;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_max_motor_force(PN_stdfloat force) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_maxMotorForce = (btScalar)force;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_max_limit_force(PN_stdfloat force) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_maxLimitForce = (btScalar)force;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_damping(PN_stdfloat damping) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_damping = (btScalar)damping;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_softness(PN_stdfloat softness) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_limitSoftness = (btScalar)softness;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_bounce(PN_stdfloat bounce) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_bounce = (btScalar)bounce;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_normal_cfm(PN_stdfloat cfm) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_normalCFM = (btScalar)cfm;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_stop_cfm(PN_stdfloat cfm) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_stopCFM = (btScalar)cfm;
}

/**
 *
 */
void BulletRotationalLimitMotor::
set_stop_erp(PN_stdfloat erp) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motor.m_stopERP = (btScalar)erp;
}

/**
 * Retrieves the current value of angle: 0 = free, 1 = at low limit, 2 = at
 * high limit.
 */
int BulletRotationalLimitMotor::
get_current_limit() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _motor.m_currentLimit;
}

/**
 *
 */
PN_stdfloat BulletRotationalLimitMotor::
get_current_error() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_motor.m_currentLimitError;
}

/**
 *
 */
PN_stdfloat BulletRotationalLimitMotor::
get_current_position() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_motor.m_currentPosition;
}

/**
 *
 */
PN_stdfloat BulletRotationalLimitMotor::
get_accumulated_impulse() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_motor.m_accumulatedImpulse;
}
