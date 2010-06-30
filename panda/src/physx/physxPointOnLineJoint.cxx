// Filename: physxPointOnLineJoint.cxx
// Created by:  enn0x (02Oct09)
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

#include "physxPointOnLineJoint.h"
#include "physxPointOnLineJointDesc.h"

TypeHandle PhysxPointOnLineJoint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxPointOnLineJoint::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxPointOnLineJoint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isPointOnLineJoint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPointOnLineJoint::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxPointOnLineJoint::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxPointOnLineJoint::save_to_desc
//       Access : Published
//  Description : Saves the state of the joint object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxPointOnLineJoint::
save_to_desc(PhysxPointOnLineJointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxPointOnLineJoint::load_from_desc
//       Access : Published
//  Description : Loads the entire state of the joint from a 
//                descriptor with a single call.
////////////////////////////////////////////////////////////////////
void PhysxPointOnLineJoint::
load_from_desc(const PhysxPointOnLineJointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}

