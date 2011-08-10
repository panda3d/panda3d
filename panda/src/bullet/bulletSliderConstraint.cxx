// Filename: bulletSliderConstraint.cxx
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

#include "bulletSliderConstraint.h"
#include "bulletRigidBodyNode.h"

#include "deg_2_rad.h"

TypeHandle BulletSliderConstraint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSliderConstraint::
BulletSliderConstraint(const BulletRigidBodyNode *node_a, 
                       CPT(TransformState) frame_a,
                       bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  _constraint = new btSliderConstraint(*ptr_a, trans_a, use_frame_a);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSliderConstraint::
BulletSliderConstraint(const BulletRigidBodyNode *node_a,
                       const BulletRigidBodyNode *node_b,
                       CPT(TransformState) frame_a,
                       CPT(TransformState) frame_b,
                       bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btTransform trans_b = TransformState_to_btTrans(frame_b);

  _constraint = new btSliderConstraint(*ptr_a, *ptr_b, trans_a, trans_b, use_frame_a);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btTypedConstraint *BulletSliderConstraint::
ptr() const {

  return _constraint;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_lower_linear_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_lower_linear_limit() const {

  return _constraint->getLowerLinLimit();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_upper_linear_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_upper_linear_limit() const {

  return _constraint->getUpperLinLimit();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_lower_angular_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_lower_angular_limit() const {

  return rad_2_deg(_constraint->getLowerAngLimit());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_upper_angular_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_upper_angular_limit() const {

  return rad_2_deg(_constraint->getUpperAngLimit());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_lower_linear_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_lower_linear_limit(float value) {

  _constraint->setLowerLinLimit(value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_upper_linear_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_upper_linear_limit(float value) {

  _constraint->setUpperLinLimit(value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_lower_angular_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_lower_angular_limit(float value) {

  _constraint->setLowerAngLimit(deg_2_rad(value));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_upper_angular_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_upper_angular_limit(float value) {

  _constraint->setUpperAngLimit(deg_2_rad(value));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_linear_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_linear_pos() const {

  return _constraint->getLinearPos();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_angular_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_angular_pos() const {

  return _constraint->getAngularPos();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_powered_linear_motor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_powered_linear_motor(bool on) {

  _constraint->setPoweredLinMotor(on);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_target_linear_motor_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_target_linear_motor_velocity(float target_velocity) {

  _constraint->setTargetLinMotorVelocity(target_velocity);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_max_linear_motor_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_max_linear_motor_force(float max_force) {

  _constraint->setMaxLinMotorForce(max_force);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_powered_linear_motor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletSliderConstraint::
get_powered_linear_motor() const {

  return _constraint->getPoweredLinMotor();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_target_linear_motor_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_target_linear_motor_velocity() const {

  return _constraint->getTargetLinMotorVelocity();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_max_linear_motor_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_max_linear_motor_force() const {

  return _constraint->getMaxLinMotorForce();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_powered_angular_motor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_powered_angular_motor(bool on) {

  _constraint->setPoweredAngMotor(on);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_target_angular_motor_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_target_angular_motor_velocity(float target_velocity) {

  _constraint->setTargetAngMotorVelocity(target_velocity);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::set_max_angular_motor_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSliderConstraint::
set_max_angular_motor_force(float max_force) {

  _constraint->setMaxAngMotorForce(max_force);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_powered_angular_motor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletSliderConstraint::
get_powered_angular_motor() const {

  return _constraint->getPoweredAngMotor();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_target_angular_motor_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_target_angular_motor_velocity() const {

  return _constraint->getTargetAngMotorVelocity();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_max_angular_motor_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_max_angular_motor_force() const {

  return _constraint->getMaxAngMotorForce();
}

