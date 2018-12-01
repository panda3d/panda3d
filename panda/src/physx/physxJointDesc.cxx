/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointDesc.cxx
 * @author enn0x
 * @date 2009-09-28
 */

#include "physxJointDesc.h"
#include "physxManager.h"

/**
 * Sets a possible debug name.
 */
void PhysxJointDesc::
set_name(const char *name) {

  _name = name ? name : "";
  ptr()->name = _name.c_str();
}

/**
 * Set a possible debug name.
 */
void PhysxJointDesc::
set_max_force(float force) {

  ptr()->maxForce = force;
}

/**
 * Set the maximum angular force (torque) that the joint can withstand before
 * breaking, must be positive.
 */
void PhysxJointDesc::
set_max_torque(float torque) {

  ptr()->maxTorque = torque;
}

/**
 * Set the extrapolation factor for solving joint constraints.
 */
void PhysxJointDesc::
set_solver_extrapolation_factor(float factor) {

  ptr()->solverExtrapolationFactor = factor;
}

/**
 * Set the two actors connected by the joint.  idx must be either 0 or 1.
 */
void PhysxJointDesc::
set_actor(unsigned int idx, const PhysxActor &actor) {

  nassertv_always(idx < 2);
  ptr()->actor[idx] = actor.ptr();
}

/**
 * Set the X axis of joint space, in actor[i]'s space, orthogonal to
 * localAxis[i]. idx must be either 0 or 1.
 */
void PhysxJointDesc::
set_local_normal(unsigned int idx, const LVector3f &normal) {

  nassertv_always(idx < 2);
  ptr()->localNormal[idx] = PhysxManager::vec3_to_nxVec3(normal);
}

/**
 * Set the Z axis of joint space, in actor[i]'s space.  This is the primary
 * axis of the joint.  idx must be either 0 or 1.
 */
void PhysxJointDesc::
set_local_axis(unsigned int idx, const LVector3f &axis) {

  nassertv_always(idx < 2);
  ptr()->localAxis[idx] = PhysxManager::vec3_to_nxVec3(axis);
}

/**
 * Set the attachment point of joint in actor[i]'s space.  idx must be either
 * 0 or 1.
 */
void PhysxJointDesc::
set_local_anchor(unsigned int idx, const LPoint3f &anchor) {

  nassertv_always(idx < 2);
  ptr()->localAnchor[idx] = PhysxManager::point3_to_nxVec3(anchor);
}

/**
 * Set or clear a single JointFlag.
 */
void PhysxJointDesc::
set_joint_flag(PhysxJointFlag flag, bool value) {

  if (value == true) {
    ptr()->jointFlags |= flag;
  }
  else {
    ptr()->jointFlags &= ~(flag);
  }
}

/**
 * Set the local axis/normal using a world space axis.
 */
void PhysxJointDesc::
set_global_axis(const LVector3f &axis) {

  ptr()->setGlobalAxis(PhysxManager::vec3_to_nxVec3(axis));
}

/**
 * Set the anchor using a world space point.
 */
void PhysxJointDesc::
set_global_anchor(const LPoint3f &anchor) {

  ptr()->setGlobalAnchor(PhysxManager::point3_to_nxVec3(anchor));
}

/**
 *
 */
const char *PhysxJointDesc::
get_name() const {

  return ptr()->name;
}

/**
 *
 */
float PhysxJointDesc::
get_max_force() const {

  return ptr()->maxForce;
}

/**
 *
 */
float PhysxJointDesc::
get_max_torque() const {

  return ptr()->maxTorque;
}

/**
 *
 */
float PhysxJointDesc::
get_solver_extrapolation_factor() const {

  return ptr()->solverExtrapolationFactor;
}

/**
 *
 */
LVector3f PhysxJointDesc::
get_local_normal(unsigned int idx) const {

  nassertr_always(idx < 2, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(ptr()->localNormal[idx]);
}

/**
 *
 */
LVector3f PhysxJointDesc::
get_local_axis(unsigned int idx) const {

  nassertr_always(idx < 2, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(ptr()->localAxis[idx]);
}

/**
 *
 */
LPoint3f PhysxJointDesc::
get_local_anchor(unsigned int idx) const {

  nassertr_always(idx < 2, LPoint3f::zero());
  return PhysxManager::nxVec3_to_point3(ptr()->localAnchor[idx]);
}

/**
 *
 */
bool PhysxJointDesc::
get_joint_flag(const PhysxJointFlag flag) const {

  return (ptr()->jointFlags & flag) ? true : false;
}

/**
 *
 */
PhysxActor *PhysxJointDesc::
get_actor(unsigned int idx) const {

  nassertr_always(idx < 2, nullptr);

  NxActor *actorPtr = ptr()->actor[idx];
  if (actorPtr == nullptr) {
    return nullptr;
  }
  else {
    return (PhysxActor *)(actorPtr->userData);
  }
}
