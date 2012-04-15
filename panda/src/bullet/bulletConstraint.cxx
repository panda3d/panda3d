// Filename: bulletConstraint.cxx
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

#include "bulletConstraint.h"
#include "bulletRigidBodyNode.h"

TypeHandle BulletConstraint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletConstraint::enable_feedback
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConstraint::
enable_feedback(bool value) {

  ptr()->enableFeedback(value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConstraint::get_applied_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletConstraint::
get_applied_impulse() const {

  return (PN_stdfloat)ptr()->getAppliedImpulse();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConstraint::set_dbg_draw_size
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConstraint::
set_debug_draw_size(PN_stdfloat size) {

  ptr()->setDbgDrawSize((btScalar)size);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConstraint::get_dbg_draw_size
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletConstraint::
get_debug_draw_size() {

  return (PN_stdfloat)ptr()->getDbgDrawSize();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConstraint::get_rigid_body_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletRigidBodyNode *BulletConstraint::
get_rigid_body_a() {

  return (BulletRigidBodyNode *)ptr()->getRigidBodyA().getUserPointer();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConstraint::get_rigid_body_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletRigidBodyNode *BulletConstraint::
get_rigid_body_b() {

  return (BulletRigidBodyNode *)ptr()->getRigidBodyB().getUserPointer();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConstraint::set_param
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConstraint::
set_param(ConstraintParam num, PN_stdfloat value, int axis) {

  ptr()->setParam((btConstraintParams)num, (btScalar)value, axis);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConstraint::get_param
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletConstraint::
get_param(ConstraintParam num, int axis) {

  return (PN_stdfloat)ptr()->getParam((btConstraintParams)num, axis);
}

