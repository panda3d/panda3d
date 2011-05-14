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
                          const TransformState &frame_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = LMatrix4f_to_btTrans(frame_a.get_mat());

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
                          const TransformState &frame_a,
                          const TransformState &frame_b) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btTransform trans_a = LMatrix4f_to_btTrans(frame_a.get_mat());

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btTransform trans_b = LMatrix4f_to_btTrans(frame_b.get_mat());

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
set_limit(int index, float value) {
 
  value = deg_2_rad(value);

  _constraint->setLimit(index, value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeTwistConstraint::set_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConeTwistConstraint::
set_limit(float swing1, float swing2, float twist, float softness, float bias, float relaxation) {

  swing1 = deg_2_rad(swing1);
  swing2 = deg_2_rad(swing2);
  twist  = deg_2_rad(twist);

  _constraint->setLimit(swing1, swing2, twist, softness, bias, relaxation);
}

