// Filename: physxJointDesc.cxx
// Created by:  enn0x (28Sep09)
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

#include "physxJointDesc.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_name
//       Access: Published
//  Description: Sets a possible debug name.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_name(const char *name) {

  _name = name ? name : "";
  ptr()->name = _name.c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_max_force
//       Access: Published
//  Description: Set a possible debug name.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_max_force(float force) {

  ptr()->maxForce = force;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_max_torque
//       Access: Published
//  Description: Set the maximum angular force (torque) that the
//               joint can withstand before breaking, must be
//               positive.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_max_torque(float torque) {

  ptr()->maxTorque = torque;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_solver_extrapolation_factor
//       Access: Published
//  Description: Set the extrapolation factor for solving joint
//               constraints.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_solver_extrapolation_factor(float factor) {

  ptr()->solverExtrapolationFactor = factor;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_actor
//       Access: Published
//  Description: Set the two actors connected by the joint.
//               idx must be either 0 or 1.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_actor(unsigned int idx, const PhysxActor &actor) {

  nassertv_always(idx < 2);
  ptr()->actor[idx] = actor.ptr();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_local_normal
//       Access: Published
//  Description: Set the X axis of joint space, in actor[i]'s space,
//               orthogonal to localAxis[i].
//               idx must be either 0 or 1.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_local_normal(unsigned int idx, const LVector3f &normal) {

  nassertv_always(idx < 2);
  ptr()->localNormal[idx] = PhysxManager::vec3_to_nxVec3(normal);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_local_axis
//       Access: Published
//  Description: Set the Z axis of joint space, in actor[i]'s space.
//               This is the primary axis of the joint. 
//               idx must be either 0 or 1.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_local_axis(unsigned int idx, const LVector3f &axis) {

  nassertv_always(idx < 2);
  ptr()->localAxis[idx] = PhysxManager::vec3_to_nxVec3(axis);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_local_anchor
//       Access: Published
//  Description: Set the attachment point of joint in actor[i]'s
//               space. idx must be either 0 or 1.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_local_anchor(unsigned int idx, const LPoint3f &anchor) {

  nassertv_always(idx < 2);
  ptr()->localAnchor[idx] = PhysxManager::point3_to_nxVec3(anchor);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_joint_flag
//       Access: Published
//  Description: Set or clear a single JointFlag.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_joint_flag(PhysxJointFlag flag, bool value) {

  if (value == true) {
    ptr()->jointFlags |= flag;
  }
  else {
    ptr()->jointFlags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_global_axis
//       Access: Published
//  Description: Set the local axis/normal using a world space axis.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_global_axis(const LVector3f &axis) {

  ptr()->setGlobalAxis(PhysxManager::vec3_to_nxVec3(axis));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::set_global_anchor
//       Access: Published
//  Description: Set the anchor using a world space point.
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_global_anchor(const LPoint3f &anchor) {

  ptr()->setGlobalAnchor(PhysxManager::point3_to_nxVec3(anchor));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
const char *PhysxJointDesc::
get_name() const {

  return ptr()->name;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_max_force
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointDesc::
get_max_force() const {

  return ptr()->maxForce;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_max_torque
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointDesc::
get_max_torque() const {

  return ptr()->maxTorque;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_solver_extrapolation_factor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxJointDesc::
get_solver_extrapolation_factor() const {

  return ptr()->solverExtrapolationFactor;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_local_normal
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LVector3f PhysxJointDesc::
get_local_normal(unsigned int idx) const {

  nassertr_always(idx < 2, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(ptr()->localNormal[idx]);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_local_axis
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LVector3f PhysxJointDesc::
get_local_axis(unsigned int idx) const {

  nassertr_always(idx < 2, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(ptr()->localAxis[idx]);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_local_anchor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3f PhysxJointDesc::
get_local_anchor(unsigned int idx) const {

  nassertr_always(idx < 2, LPoint3f::zero());
  return PhysxManager::nxVec3_to_point3(ptr()->localAnchor[idx]);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_joint_flag
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxJointDesc::
get_joint_flag(const PhysxJointFlag flag) const {

  return (ptr()->jointFlags & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDesc::get_actor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxActor *PhysxJointDesc::
get_actor(unsigned int idx) const {

  nassertr_always(idx < 2, NULL);

  NxActor *actorPtr = ptr()->actor[idx];
  if (actorPtr == NULL) {
    return NULL;
  }
  else {
    return (PhysxActor *)(actorPtr->userData);
  }
}

