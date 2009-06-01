// Filename: physxBodyDesc.cxx
// Created by:  pratt (Apr 7, 2006)
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

#include "physxBodyDesc.h"

#include "luse.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxBodyDesc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxBodyDesc::
PhysxBodyDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function : get_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBodyDesc::
get_angular_velocity() const {
  return PhysxManager::nxVec3_to_lVecBase3(nBodyDesc.angularVelocity);
}

////////////////////////////////////////////////////////////////////
//     Function : get_linear_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBodyDesc::
get_linear_velocity() const {
  return PhysxManager::nxVec3_to_lVecBase3(nBodyDesc.linearVelocity);
}

////////////////////////////////////////////////////////////////////
//     Function : get_mass_local_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxBodyDesc::
get_mass_local_pose() const {
  return PhysxManager::nxMat34_to_lMatrix4(nBodyDesc.massLocalPose);
}

////////////////////////////////////////////////////////////////////
//     Function : get_mass_space_inertia
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBodyDesc::
get_mass_space_inertia() const {
  return PhysxManager::nxVec3_to_lVecBase3(nBodyDesc.massSpaceInertia);
}

////////////////////////////////////////////////////////////////////
//     Function : set_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_angular_velocity(LVecBase3f value) {
  nBodyDesc.angularVelocity = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_linear_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_linear_velocity(LVecBase3f value) {
  nBodyDesc.linearVelocity = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_mass_local_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_mass_local_pose(LMatrix4f value) {
  nBodyDesc.massLocalPose = PhysxManager::lMatrix4_to_nxMat34(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_mass_space_inertia
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_mass_space_inertia(LVecBase3f value) {
  nBodyDesc.massSpaceInertia = PhysxManager::lVecBase3_to_nxVec3(value);
}

#endif // HAVE_PHYSX

