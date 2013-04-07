// Filename: bulletSphericalConstraint.cxx
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

#include "bulletSphericalConstraint.h"
#include "bulletRigidBodyNode.h"

TypeHandle BulletSphericalConstraint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletSphericalConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSphericalConstraint::
BulletSphericalConstraint(const BulletRigidBodyNode *node_a, 
                          const LPoint3 &pivot_a) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btVector3 pos_a = LVecBase3_to_btVector3(pivot_a);

  _constraint = new btPoint2PointConstraint(*ptr_a, pos_a);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphericalConstraint::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSphericalConstraint::
BulletSphericalConstraint(const BulletRigidBodyNode *node_a,
                          const BulletRigidBodyNode *node_b,
                          const LPoint3 &pivot_a,
                          const LPoint3 &pivot_b) {

  btRigidBody *ptr_a = btRigidBody::upcast(node_a->get_object());
  btVector3 pos_a = LVecBase3_to_btVector3(pivot_a);

  btRigidBody *ptr_b = btRigidBody::upcast(node_b->get_object());
  btVector3 pos_b = LVecBase3_to_btVector3(pivot_b);

  _constraint = new btPoint2PointConstraint(*ptr_a, *ptr_b, pos_a, pos_b);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphericalConstraint::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btTypedConstraint *BulletSphericalConstraint::
ptr() const {

  return _constraint;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphericalConstraint::set_pivot_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSphericalConstraint::
set_pivot_a(const LPoint3 &pivot_a) {

  nassertv(!pivot_a.is_nan());
  _constraint->setPivotA(LVecBase3_to_btVector3(pivot_a));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphericalConstraint::set_pivot_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSphericalConstraint::
set_pivot_b(const LPoint3 &pivot_b) {

  nassertv(!pivot_b.is_nan());
  _constraint->setPivotB(LVecBase3_to_btVector3(pivot_b));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphericalConstraint::set_pivot_in_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletSphericalConstraint::
get_pivot_in_a() const {

  return btVector3_to_LPoint3(_constraint->getPivotInA());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphericalConstraint::set_pivot_in_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletSphericalConstraint::
get_pivot_in_b() const {

  return btVector3_to_LPoint3(_constraint->getPivotInB());
}

