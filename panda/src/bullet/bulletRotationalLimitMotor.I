// Filename: bulletRotationalLimitMotor.I
// Created by:  enn0x (03Mar13)
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


////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::is_limited
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletRotationalLimitMotor::
is_limited() const {

  return _motor.isLimited();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_motor_enabled
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_motor_enabled(bool enabled) {

  _motor.m_enableMotor = enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::get_motor_enabled
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletRotationalLimitMotor::
get_motor_enabled() const {

  return _motor.m_enableMotor;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_low_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_low_limit(PN_stdfloat limit) {

  _motor.m_loLimit = (btScalar)limit;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_high_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_high_limit(PN_stdfloat limit) {

  _motor.m_hiLimit = (btScalar)limit;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_target_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_target_velocity(PN_stdfloat velocity) {

  _motor.m_targetVelocity = (btScalar)velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_max_motor_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_max_motor_force(PN_stdfloat force) {

  _motor.m_maxMotorForce = (btScalar)force;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_max_limit_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_max_limit_force(PN_stdfloat force) {

  _motor.m_maxLimitForce = (btScalar)force;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_damping(PN_stdfloat damping) {

  _motor.m_damping = (btScalar)damping;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_softness
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_softness(PN_stdfloat softness) {

  _motor.m_limitSoftness = (btScalar)softness;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_bounce
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_bounce(PN_stdfloat bounce) {

  _motor.m_bounce = (btScalar)bounce;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_normal_cfm
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_normal_cfm(PN_stdfloat cfm) {

  _motor.m_normalCFM = (btScalar)cfm;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_stop_cfm
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_stop_cfm(PN_stdfloat cfm) {

  _motor.m_stopCFM = (btScalar)cfm;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::set_stop_erp
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRotationalLimitMotor::
set_stop_erp(PN_stdfloat erp) {

  _motor.m_stopERP = (btScalar)erp;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::get_current_limit
//       Access: Published
//  Description: Retrieves the current value of angle:
//               0 = free,
//               1 = at low limit,
//               2 = at high limit.
////////////////////////////////////////////////////////////////////
INLINE int BulletRotationalLimitMotor::
get_current_limit() const {

  return _motor.m_currentLimit;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::get_current_error
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletRotationalLimitMotor::
get_current_error() const {

  return (PN_stdfloat)_motor.m_currentLimitError;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::get_current_position
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletRotationalLimitMotor::
get_current_position() const {

  return (PN_stdfloat)_motor.m_currentPosition;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::get_accumulated_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletRotationalLimitMotor::
get_accumulated_impulse() const {

  return (PN_stdfloat)_motor.m_accumulatedImpulse;
}

