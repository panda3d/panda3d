// Filename: physxD6Joint.cxx
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

#include "physxD6Joint.h"
#include "physxD6JointDesc.h"

TypeHandle PhysxD6Joint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6Joint::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isD6Joint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6Joint::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxD6Joint::save_to_desc
//       Access : Published
//  Description : Saves the state of the joint object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
save_to_desc(PhysxD6JointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxD6Joint::load_from_desc
//       Access : Published
//  Description : Loads the entire state of the joint from a 
//                descriptor with a single call.
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
load_from_desc(const PhysxD6JointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6Joint::set_drive_angular_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
set_drive_angular_velocity(const LVector3f &v) {

  nassertv(_error_type == ET_ok);
  _ptr->setDriveAngularVelocity(PhysxManager::vec3_to_nxVec3(v));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6Joint::set_drive_linear_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
set_drive_linear_velocity(const LVector3f &v) {

  nassertv(_error_type == ET_ok);
  _ptr->setDriveLinearVelocity(PhysxManager::vec3_to_nxVec3(v));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6Joint::set_drive_orientation
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
set_drive_orientation(const LQuaternionf &quat) {

  nassertv(_error_type == ET_ok);
  _ptr->setDriveOrientation(PhysxManager::quat_to_nxQuat(quat));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6Joint::set_drive_position
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
set_drive_position(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);
  _ptr->setDrivePosition(PhysxManager::point3_to_nxVec3(pos));
}

