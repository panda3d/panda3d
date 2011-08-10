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
                        CPT(TransformState) frame_a,
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
                        CPT(TransformState) frame_a,
                        CPT(TransformState) frame_b,
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
//     Function: BulletGenericConstraint::set_linear_lower_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGenericConstraint::
set_linear_lower_limit(const LPoint3f &limit) {

  nassertv(!limit.is_nan());
  _constraint->setLinearLowerLimit(LVecBase3f_to_btVector3(limit));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::set_linear_upper_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGenericConstraint::
set_linear_upper_limit(const LPoint3f &limit) {

  nassertv(!limit.is_nan());
  _constraint->setLinearUpperLimit(LVecBase3f_to_btVector3(limit));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::set_angular_lower_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGenericConstraint::
set_angular_lower_limit(const LVector3f &limit) {

  nassertv(!limit.is_nan());
  _constraint->setAngularLowerLimit(LVecBase3f_to_btVector3(limit));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGenericConstraint::set_angular_upper_limit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGenericConstraint::
set_angular_upper_limit(const LVector3f &limit) {

  nassertv(!limit.is_nan());
  _constraint->setAngularUpperLimit(LVecBase3f_to_btVector3(limit));
}

