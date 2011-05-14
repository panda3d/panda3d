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
                       const TransformState &frame_a,
                       bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = LMatrix4f_to_btTrans(frame_a.get_mat());

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
                       const TransformState &frame_a,
                       const TransformState &frame_b,
                       bool use_frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = LMatrix4f_to_btTrans(frame_a.get_mat());

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btTransform trans_b = LMatrix4f_to_btTrans(frame_b.get_mat());

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
get_lower_linear_limit() {

  return _constraint->getLowerLinLimit();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_upper_linear_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_upper_linear_limit() {

  return _constraint->getUpperLinLimit();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_lower_angular_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_lower_angular_limit() {

  return rad_2_deg(_constraint->getLowerAngLimit());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_upper_angular_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSliderConstraint::
get_upper_angular_limit() {

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

