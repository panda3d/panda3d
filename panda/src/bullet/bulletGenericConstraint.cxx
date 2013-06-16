// Filename: bulletGenericConstraint.cxx
// Created by:  enn0x (02Mar10)
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

#include "bulletGenericConstraint.h"
#include "bulletRigidBodyNode.h"

TypeHandle BulletGenericConstraint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletGenericConstraint::
BulletGenericConstraint(const BulletRigidBodyNode *node_a, 
                        const TransformState *frame_a,
                        bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = TransformState_to_btTrans(frame_a);

  _constraint = new btGeneric6DofConstraint(*ptr_a, trans_a, use_frame_a);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btTypedConstraint *BulletGenericConstraint::
ptr() const {

  return _constraint;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::get_axis
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3 BulletGenericConstraint::
get_axis(int axis) const {

  nassertr(axis >= 0, LVector3::zero());
  nassertr(axis <= 3, LVector3::zero());

  _constraint->buildJacobian();
  return btVector3_to_LVector3(_constraint->getAxis(axis));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::get_pivot
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletGenericConstraint::
get_pivot(int axis) const {

  nassertr(axis >= 0, 0.0f);
  nassertr(axis <= 3, 0.0f);

  _constraint->buildJacobian();
  return _constraint->getRelativePivotPosition(axis);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::get_angle
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletGenericConstraint::
get_angle(int axis) const {

  nassertr(axis >= 0, 0.0f);
  nassertr(axis <= 3, 0.0f);

  _constraint->buildJacobian();
  return _constraint->getAngle(axis);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::set_linear_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGenericConstraint::
set_linear_limit(int axis, PN_stdfloat low, PN_stdfloat high) {

  nassertv(axis >= 0);
  nassertv(axis <= 3);

  _constraint->buildJacobian();
  _constraint->setLimit(axis, low, high);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::set_angular_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGenericConstraint::
set_angular_limit(int axis, PN_stdfloat low, PN_stdfloat high) {

  nassertv(axis >= 0);
  nassertv(axis <= 3);

  low  = deg_2_rad(low);
  high = deg_2_rad(high);

  _constraint->buildJacobian();
  _constraint->setLimit(axis + 3, low, high);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::get_rotational_limit_motor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletRotationalLimitMotor BulletGenericConstraint::
get_rotational_limit_motor(int axis) {

  return BulletRotationalLimitMotor(*_constraint->getRotationalLimitMotor(axis));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::get_translational_limit_motor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletTranslationalLimitMotor BulletGenericConstraint::
get_translational_limit_motor() {

  return BulletTranslationalLimitMotor(*_constraint->getTranslationalLimitMotor());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::set_frames
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGenericConstraint::
set_frames(const TransformState *ts_a, const TransformState *ts_b) {

  btTransform frame_a = TransformState_to_btTrans(ts_a);
  btTransform frame_b = TransformState_to_btTrans(ts_b);

  _constraint->setFrames(frame_a, frame_b);
}

