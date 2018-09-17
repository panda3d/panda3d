/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConeTwistConstraint.cxx
 * @author enn0x
 * @date 2010-03-01
 */

#include "bulletConeTwistConstraint.h"

#include "bulletRigidBodyNode.h"
#include "bulletWorld.h"

#include "deg_2_rad.h"

TypeHandle BulletConeTwistConstraint::_type_handle;

/**
 *
 */
BulletConeTwistConstraint::
BulletConeTwistConstraint(const BulletRigidBodyNode *node_a,
                          const TransformState *frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  _constraint = new btConeTwistConstraint(*ptr_a, trans_a);
}

/**
 *
 */
BulletConeTwistConstraint::
BulletConeTwistConstraint(const BulletRigidBodyNode *node_a,
                          const BulletRigidBodyNode *node_b,
                          const TransformState *frame_a,
                          const TransformState *frame_b) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btTransform trans_b = TransformState_to_btTrans(frame_b);

  _constraint = new btConeTwistConstraint(*ptr_a, *ptr_b, trans_a, trans_b);
}

/**
 *
 */
btTypedConstraint *BulletConeTwistConstraint::
ptr() const {

  return _constraint;
}

/**
 *
 */
void BulletConeTwistConstraint::
set_limit(int index, PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  value = deg_2_rad(value);

  _constraint->setLimit(index, value);
}

/**
 *
 */
void BulletConeTwistConstraint::
set_limit(PN_stdfloat swing1, PN_stdfloat swing2, PN_stdfloat twist, PN_stdfloat softness, PN_stdfloat bias, PN_stdfloat relaxation) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  swing1 = deg_2_rad(swing1);
  swing2 = deg_2_rad(swing2);
  twist  = deg_2_rad(twist);

  _constraint->setLimit(swing1, swing2, twist, softness, bias, relaxation);
}

/**
 *
 */
void BulletConeTwistConstraint::
set_damping(PN_stdfloat damping) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setDamping(damping);
}

/**
 *
 */
PN_stdfloat BulletConeTwistConstraint::
get_fix_threshold() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _constraint->getFixThresh();
}

/**
 *
 */
void BulletConeTwistConstraint::
set_fix_threshold(PN_stdfloat threshold) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setFixThresh(threshold);
}

/**
 *
 */
void BulletConeTwistConstraint::
enable_motor(bool enable) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->enableMotor(enable);
}

/**
 *
 */
void BulletConeTwistConstraint::
set_max_motor_impulse(PN_stdfloat max_impulse) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setMaxMotorImpulse(max_impulse);
}

/**
 *
 */
void BulletConeTwistConstraint::
set_max_motor_impulse_normalized(PN_stdfloat max_impulse) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setMaxMotorImpulseNormalized(max_impulse);
}

/**
 *
 */
void BulletConeTwistConstraint::
set_motor_target(const LQuaternion &quat) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setMotorTarget(LQuaternion_to_btQuat(quat));
}

/**
 *
 */
void BulletConeTwistConstraint::
set_motor_target_in_constraint_space(const LQuaternion &quat) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _constraint->setMotorTargetInConstraintSpace(LQuaternion_to_btQuat(quat));
}

/**
 *
 */
void BulletConeTwistConstraint::
set_frames(const TransformState *ts_a, const TransformState *ts_b) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btTransform frame_a = TransformState_to_btTrans(ts_a);
  btTransform frame_b = TransformState_to_btTrans(ts_b);

  _constraint->setFrames(frame_a, frame_b);
}

/**
 *
 */
CPT(TransformState) BulletConeTwistConstraint::
get_frame_a() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_TransformState(_constraint->getAFrame());
}

/**
 *
 */
CPT(TransformState) BulletConeTwistConstraint::
get_frame_b() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_TransformState(_constraint->getBFrame());
}
