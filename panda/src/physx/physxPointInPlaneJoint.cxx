// Filename: physxPointInPlaneJoint.cxx
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

#include "physxPointInPlaneJoint.h"
#include "physxPointInPlaneJointDesc.h"

TypeHandle PhysxPointInPlaneJoint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxPointInPlaneJoint::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxPointInPlaneJoint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isPointInPlaneJoint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPointInPlaneJoint::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxPointInPlaneJoint::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxPointInPlaneJoint::save_to_desc
//       Access : Published
//  Description : Saves the state of the joint object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxPointInPlaneJoint::
save_to_desc(PhysxPointInPlaneJointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxPointInPlaneJoint::load_from_desc
//       Access : Published
//  Description : Loads the entire state of the joint from a 
//                descriptor with a single call.
////////////////////////////////////////////////////////////////////
void PhysxPointInPlaneJoint::
load_from_desc(const PhysxPointInPlaneJointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}

