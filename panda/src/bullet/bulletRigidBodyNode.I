// Filename: bulletRigidBodyNode.I
// Created by:  enn0x (19Nov10)
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


////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletRigidBodyNode::
~BulletRigidBodyNode() {

  delete _rigid;
  delete _motion;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_linear_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRigidBodyNode::
set_linear_damping(PN_stdfloat value) {

  _rigid->setDamping(value, _rigid->getAngularDamping());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_angular_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRigidBodyNode::
set_angular_damping(PN_stdfloat value) {

  _rigid->setDamping(_rigid->getLinearDamping(), value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_linear_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletRigidBodyNode::
get_linear_damping() const {

  return (PN_stdfloat)_rigid->getLinearDamping();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_angular_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletRigidBodyNode::
get_angular_damping() const {

  return (PN_stdfloat)_rigid->getAngularDamping();
}

