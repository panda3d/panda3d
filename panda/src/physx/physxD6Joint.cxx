// Filename: physxD6Joint.cxx
// Created by:  pratt (Jun 16, 2006)
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

#ifdef HAVE_PHYSX

#include "physxD6Joint.h"

#include "luse.h"
#include "physxD6JointDesc.h"

TypeHandle PhysxD6Joint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : load_from_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
load_from_desc(const PhysxD6JointDesc & desc) {
  nassertv(nD6Joint != NULL);

  nD6Joint->loadFromDesc(desc.nD6JointDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : save_to_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
save_to_desc(PhysxD6JointDesc & desc) {
  nassertv(nD6Joint != NULL);

  nD6Joint->saveToDesc(desc.nD6JointDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_drive_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
set_drive_angular_velocity(const LVecBase3f & ang_vel) {
  nassertv(nD6Joint != NULL);

  nD6Joint->setDriveAngularVelocity(PhysxManager::lVecBase3_to_nxVec3(ang_vel));
}

////////////////////////////////////////////////////////////////////
//     Function : set_drive_linear_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
set_drive_linear_velocity(const LVecBase3f & lin_vel) {
  nassertv(nD6Joint != NULL);

  nD6Joint->setDriveLinearVelocity(PhysxManager::lVecBase3_to_nxVec3(lin_vel));
}

////////////////////////////////////////////////////////////////////
//     Function : set_drive_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
set_drive_orientation(const LQuaternionf & orientation) {
  nassertv(nD6Joint != NULL);

  nD6Joint->setDriveOrientation(PhysxManager::lQuaternion_to_nxQuat(orientation));
}

////////////////////////////////////////////////////////////////////
//     Function : set_drive_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxD6Joint::
set_drive_position(const LVecBase3f & position) {
  nassertv(nD6Joint != NULL);

  nD6Joint->setDrivePosition(PhysxManager::lVecBase3_to_nxVec3(position));
}

#endif // HAVE_PHYSX

