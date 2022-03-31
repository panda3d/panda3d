/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSliderConstraint.cxx
 * @author enn0x
 * @date 2010-03-01
 */

#include "bulletSliderConstraint.h"

#include "bulletRigidBodyNode.h"
#include "bulletWorld.h"

#include "deg_2_rad.h"

TypeHandle BulletSliderConstraint::_type_handle;

/**
 *
 */
BulletSliderConstraint::
BulletSliderConstraint(const BulletRigidBodyNode *node_a,
                       const TransformState *frame_a,
                       bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  _constraint = new btSliderConstraint(*ptr_a, trans_a, use_frame_a);
}

/**
 *
 */
BulletSliderConstraint::
BulletSliderConstraint(const BulletRigidBodyNode *node_a,
                       const BulletRigidBodyNode *node_b,
                       const TransformState *frame_a,
                       const TransformState *frame_b,
                       bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btTransform trans_b = TransformState_to_btTrans(frame_b);

  _constraint = new btSliderConstraint(*ptr_a, *ptr_b, trans_a, trans_b, use_frame_a);
}

/**
 *
 */
btTypedConstraint *BulletSliderConstraint::
ptr() const {

  return _constraint;
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_lower_linear_limit() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_constraint->getLowerLinLimit();
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_upper_linear_limit() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_constraint->getUpperLinLimit();
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_lower_angular_limit() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return rad_2_deg(_constraint->getLowerAngLimit());
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_upper_angular_limit() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return rad_2_deg(_constraint->getUpperAngLimit());
}

/**
 *
 */
void BulletSliderConstraint::
set_lower_linear_limit(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setLowerLinLimit((btScalar)value);
}

/**
 *
 */
void BulletSliderConstraint::
set_upper_linear_limit(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setUpperLinLimit((btScalar)value);
}

/**
 *
 */
void BulletSliderConstraint::
set_lower_angular_limit(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setLowerAngLimit((btScalar)deg_2_rad(value));
}

/**
 *
 */
void BulletSliderConstraint::
set_upper_angular_limit(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setUpperAngLimit((btScalar)deg_2_rad(value));
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_linear_pos() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_constraint->getLinearPos();
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_angular_pos() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_constraint->getAngularPos();
}

/**
 *
 */
void BulletSliderConstraint::
set_powered_linear_motor(bool on) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setPoweredLinMotor(on);
}

/**
 *
 */
void BulletSliderConstraint::
set_target_linear_motor_velocity(PN_stdfloat target_velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setTargetLinMotorVelocity((btScalar)target_velocity);
}

/**
 *
 */
void BulletSliderConstraint::
set_max_linear_motor_force(PN_stdfloat max_force) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setMaxLinMotorForce((btScalar)max_force);
}

/**
 *
 */
bool BulletSliderConstraint::
get_powered_linear_motor() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _constraint->getPoweredLinMotor();
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_target_linear_motor_velocity() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_constraint->getTargetLinMotorVelocity();
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_max_linear_motor_force() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_constraint->getMaxLinMotorForce();
}

/**
 *
 */
void BulletSliderConstraint::
set_powered_angular_motor(bool on) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setPoweredAngMotor(on);
}

/**
 *
 */
void BulletSliderConstraint::
set_target_angular_motor_velocity(PN_stdfloat target_velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setTargetAngMotorVelocity((btScalar)target_velocity);
}

/**
 *
 */
void BulletSliderConstraint::
set_max_angular_motor_force(PN_stdfloat max_force) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setMaxAngMotorForce((btScalar)max_force);
}

/**
 *
 */
bool BulletSliderConstraint::
get_powered_angular_motor() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _constraint->getPoweredAngMotor();
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_target_angular_motor_velocity() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_constraint->getTargetAngMotorVelocity();
}

/**
 *
 */
PN_stdfloat BulletSliderConstraint::
get_max_angular_motor_force() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_constraint->getMaxAngMotorForce();
}

/**
 *
 */
void BulletSliderConstraint::
set_frames(const TransformState *ts_a, const TransformState *ts_b) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btTransform frame_a = TransformState_to_btTrans(ts_a);
  btTransform frame_b = TransformState_to_btTrans(ts_b);

  _constraint->setFrames(frame_a, frame_b);
}

/**
 *
 */
CPT(TransformState) BulletSliderConstraint::
get_frame_a() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_TransformState(_constraint->getFrameOffsetA());
}

/**
 *
 */
CPT(TransformState) BulletSliderConstraint::
get_frame_b() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_TransformState(_constraint->getFrameOffsetB());
}
