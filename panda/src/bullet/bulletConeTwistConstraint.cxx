// Filename: bulletConeTwistConstraint.cxx
// Created by:  enn0x (01Mar10)
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

#include "bulletConeTwistConstraint.h"
#include "bulletRigidBodyNode.h"

#include "deg_2_rad.h"

TypeHandle BulletConeTwistConstraint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletConeTwistConstraint::
BulletConeTwistConstraint(const BulletRigidBodyNode *node_a, 
                          const TransformState *frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  _constraint = new btConeTwistConstraint(*ptr_a, trans_a);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btTypedConstraint *BulletConeTwistConstraint::
ptr() const {

  return _constraint;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_limit(int index, PN_stdfloat value) {
 
  value = deg_2_rad(value);

  _constraint->setLimit(index, value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_limit(PN_stdfloat swing1, PN_stdfloat swing2, PN_stdfloat twist, PN_stdfloat softness, PN_stdfloat bias, PN_stdfloat relaxation) {

  swing1 = deg_2_rad(swing1);
  swing2 = deg_2_rad(swing2);
  twist  = deg_2_rad(twist);

  _constraint->setLimit(swing1, swing2, twist, softness, bias, relaxation);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_damping(PN_stdfloat damping) {
 
  _constraint->setDamping(damping);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::get_fix_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletConeTwistConstraint::
get_fix_threshold() const {
 
  return _constraint->getFixThresh();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_fix_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_fix_threshold(PN_stdfloat threshold) {
 
  _constraint->setFixThresh(threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::enable_motor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
enable_motor(bool enable) {
 
  _constraint->enableMotor(enable);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_max_motor_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_max_motor_impulse(PN_stdfloat max_impulse) {
 
  _constraint->setMaxMotorImpulse(max_impulse);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_max_motor_impulse_normalized
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_max_motor_impulse_normalized(PN_stdfloat max_impulse) {
 
  _constraint->setMaxMotorImpulseNormalized(max_impulse);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_motor_target
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_motor_target(const LQuaternion &quat) {
 
  _constraint->setMotorTarget(LQuaternion_to_btQuat(quat));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_motor_target_in_constraint_space
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_motor_target_in_constraint_space(const LQuaternion &quat) {
 
  _constraint->setMotorTargetInConstraintSpace(LQuaternion_to_btQuat(quat));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_frames
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_frames(const TransformState *ts_a, const TransformState *ts_b) {

  btTransform frame_a = TransformState_to_btTrans(ts_a);
  btTransform frame_b = TransformState_to_btTrans(ts_b);

  _constraint->setFrames(frame_a, frame_b);
}

