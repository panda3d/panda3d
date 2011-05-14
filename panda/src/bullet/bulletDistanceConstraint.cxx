// Filename: bulletDistanceConstraint.cxx
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

#include "bulletDistanceConstraint.h"
#include "bulletRigidBodyNode.h"

TypeHandle BulletDistanceConstraint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletDistanceConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletDistanceConstraint::
BulletDistanceConstraint(const BulletRigidBodyNode *node_a, 
                         const LPoint3f &pivot_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btVector3 pos_a = LVecBase3f_to_btVector3(pivot_a);

  _constraint = new btPoint2PointConstraint(*ptr_a, pos_a);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDistanceConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletDistanceConstraint::
BulletDistanceConstraint(const BulletRigidBodyNode *node_a,
                         const BulletRigidBodyNode *node_b,
                         const LPoint3f &pivot_a,
                         const LPoint3f &pivot_b) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btVector3 pos_a = LVecBase3f_to_btVector3(pivot_a);

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btVector3 pos_b = LVecBase3f_to_btVector3(pivot_b);

  _constraint = new btPoint2PointConstraint(*ptr_a, *ptr_b, pos_a, pos_b);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDistanceConstraint::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btTypedConstraint *BulletDistanceConstraint::
ptr() const {

  return _constraint;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDistanceConstraint::set_pivot_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletDistanceConstraint::
set_pivot_a(const LPoint3f &pivot_a) {

  nassertv(!pivot_a.is_nan());
  _constraint->setPivotA(LVecBase3f_to_btVector3(pivot_a));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDistanceConstraint::set_pivot_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletDistanceConstraint::
set_pivot_b(const LPoint3f &pivot_b) {

  nassertv(!pivot_b.is_nan());
  _constraint->setPivotA(LVecBase3f_to_btVector3(pivot_b));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDistanceConstraint::set_pivot_in_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f BulletDistanceConstraint::
get_pivot_in_a() const {

  return btVector3_to_LPoint3f(_constraint->getPivotInA());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDistanceConstraint::set_pivot_in_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f BulletDistanceConstraint::
get_pivot_in_b() const {

  return btVector3_to_LPoint3f(_constraint->getPivotInB());
}

