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

  delete _body;
  delete _motion;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_linear_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRigidBodyNode::
set_linear_damping(float value) {

  _body->setDamping(value, _body->getAngularDamping());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_angular_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletRigidBodyNode::
set_angular_damping(float value) {

  _body->setDamping(_body->getLinearDamping(), value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_linear_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float BulletRigidBodyNode::
get_linear_damping() const {

  return _body->getLinearDamping();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_angular_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float BulletRigidBodyNode::
get_angular_damping() const {

  return _body->getAngularDamping();
}

