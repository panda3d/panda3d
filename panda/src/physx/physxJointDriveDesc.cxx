/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointDriveDesc.cxx
 * @author enn0x
 * @date 2009-10-01
 */

#include "physxJointDriveDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::spring
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxJointDriveDesc::
set_spring(float spring) {

  _desc.spring = spring;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::set_damping
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxJointDriveDesc::
set_damping(float damping) {

  _desc.damping = damping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::set_force_limit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxJointDriveDesc::
set_force_limit(float forceLimit) {

  _desc.forceLimit = forceLimit;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::set_drive_type
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxJointDriveDesc::
set_drive_type(PhysxD6JointDriveType driveType) {

  _desc.driveType = (NxD6JointDriveType)driveType;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::get_spring
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointDriveDesc::
get_spring() const {

  return _desc.spring;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::get_damping
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointDriveDesc::
get_damping() const {

  return _desc.damping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::get_force_limit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointDriveDesc::
get_force_limit() const {

  return _desc.forceLimit;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::get_drive_type
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxD6JointDriveType PhysxJointDriveDesc::
get_drive_type() const {

  return (PhysxD6JointDriveType)_desc.driveType.bitField;
}

