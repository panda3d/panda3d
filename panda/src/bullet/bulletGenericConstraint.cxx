/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletGenericConstraint.cxx
 * @author enn0x
 * @date 2010-03-02
 */

#include "bulletGenericConstraint.h"

#include "bulletRigidBodyNode.h"
#include "bulletWorld.h"

TypeHandle BulletGenericConstraint::_type_handle;

/**
 *
 */
BulletGenericConstraint::
BulletGenericConstraint(const BulletRigidBodyNode *node_a,
                        const TransformState *frame_a,
                        bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  _constraint = new btGeneric6DofConstraint(*ptr_a, trans_a, use_frame_a);
}

/**
 *
 */
BulletGenericConstraint::
BulletGenericConstraint(const BulletRigidBodyNode *node_a,
                        const BulletRigidBodyNode *node_b,
                        const TransformState *frame_a,
                        const TransformState *frame_b,
                        bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btTransform trans_b = TransformState_to_btTrans(frame_b);

  _constraint = new btGeneric6DofConstraint(*ptr_a, *ptr_b, trans_a, trans_b, use_frame_a);
}

/**
 *
 */
btTypedConstraint *BulletGenericConstraint::
ptr() const {

  return _constraint;
}

/**
 *
 */
LVector3 BulletGenericConstraint::
get_axis(int axis) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(axis >= 0, LVector3::zero());
  nassertr(axis <= 3, LVector3::zero());

  _constraint->buildJacobian();
  return btVector3_to_LVector3(_constraint->getAxis(axis));
}

/**
 *
 */
PN_stdfloat BulletGenericConstraint::
get_pivot(int axis) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(axis >= 0, 0.0f);
  nassertr(axis <= 3, 0.0f);

  _constraint->buildJacobian();
  return _constraint->getRelativePivotPosition(axis);
}

/**
 *
 */
PN_stdfloat BulletGenericConstraint::
get_angle(int axis) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(axis >= 0, 0.0f);
  nassertr(axis <= 3, 0.0f);

  _constraint->buildJacobian();
  return _constraint->getAngle(axis);
}

/**
 *
 */
void BulletGenericConstraint::
set_linear_limit(int axis, PN_stdfloat low, PN_stdfloat high) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(axis >= 0);
  nassertv(axis <= 3);

  _constraint->buildJacobian();
  _constraint->setLimit(axis, low, high);
}

/**
 *
 */
void BulletGenericConstraint::
set_angular_limit(int axis, PN_stdfloat low, PN_stdfloat high) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(axis >= 0);
  nassertv(axis <= 3);

  low  = deg_2_rad(low);
  high = deg_2_rad(high);

  _constraint->buildJacobian();
  _constraint->setLimit(axis + 3, low, high);
}

/**
 *
 */
CPT(TransformState) BulletGenericConstraint::
get_frame_a() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_TransformState(_constraint->getFrameOffsetA());
}

/**
 *
 */
CPT(TransformState) BulletGenericConstraint::
get_frame_b() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_TransformState(_constraint->getFrameOffsetB());
}

/**
 *
 */
BulletRotationalLimitMotor BulletGenericConstraint::
get_rotational_limit_motor(int axis) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return BulletRotationalLimitMotor(*_constraint->getRotationalLimitMotor(axis));
}

/**
 *
 */
BulletTranslationalLimitMotor BulletGenericConstraint::
get_translational_limit_motor() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return BulletTranslationalLimitMotor(*_constraint->getTranslationalLimitMotor());
}

/**
 *
 */
void BulletGenericConstraint::
set_frames(const TransformState *ts_a, const TransformState *ts_b) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btTransform frame_a = TransformState_to_btTrans(ts_a);
  btTransform frame_b = TransformState_to_btTrans(ts_b);

  _constraint->setFrames(frame_a, frame_b);
}
