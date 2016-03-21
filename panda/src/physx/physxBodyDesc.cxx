/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBodyDesc.cxx
 * @author enn0x
 * @date 2009-09-05
 */

#include "physxBodyDesc.h"

/**
 * Set the mass of body.
 */
void PhysxBodyDesc::
set_mass(float mass) {

  _desc.mass = mass;
}

/**
 * Get the mass of body.
 */
float PhysxBodyDesc::
get_mass() const {

  return _desc.mass;
}

/**
 * Set the linear damping applied to the body.
 */
void PhysxBodyDesc::
set_linear_damping(float damping) {

  _desc.linearDamping = damping;
}

/**
 * Get the linear damping applied to the body.
 */
float PhysxBodyDesc::
get_linear_damping() const {

  return _desc.linearDamping;
}

/**
 * Set the angular damping applied to the body.
 */
void PhysxBodyDesc::
set_angular_damping(float damping) {

  _desc.angularDamping = damping;
}

/**
 * Get the angular damping applied to the body.
 */
float PhysxBodyDesc::
get_angular_damping() const {

  return _desc.angularDamping;
}

/**
 * Set the linear Velocity of the body.
 */
void PhysxBodyDesc::
set_linear_velocity(const LVector3f &velocity) {

  _desc.linearVelocity = PhysxManager::vec3_to_nxVec3(velocity);
}

/**
 * Get the linear Velocity of the body.
 */
LVector3f PhysxBodyDesc::
get_linear_velocity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.linearVelocity);
}

/**
 * Set the angular velocity of the body.
 */
void PhysxBodyDesc::
set_angular_velocity(const LVector3f &velocity) {

  _desc.angularVelocity = PhysxManager::vec3_to_nxVec3(velocity);
}

/**
 * Get the angular velocity of the body.
 */
LVector3f PhysxBodyDesc::
get_angular_velocity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.angularVelocity);
}

/**
 * Set the maximum allowed angular velocity for this body.
 */
void PhysxBodyDesc::
set_max_angular_velocity(float maximum) {

  _desc.maxAngularVelocity = maximum;
}

/**
 * Get the maximum allowed angular velocity for this body.
 */
float PhysxBodyDesc::
get_max_angular_velocity() const {

  return _desc.maxAngularVelocity;
}

/**
 * Set the maximum linear velocity at which the body can go to sleep.
 */
void PhysxBodyDesc::
set_sleep_linear_velocity(float velocity) {

  _desc.sleepLinearVelocity = velocity;
}

/**
 * Get the maximum linear velocity at which the body can go to sleep.
 */
float PhysxBodyDesc::
get_sleep_linear_velocity() const {

  return _desc.sleepLinearVelocity;
}

/**
 * Set the maximum angular velocity at which body can go to sleep.
 */
void PhysxBodyDesc::
set_sleep_angular_velocity(float velocity) {

  _desc.sleepAngularVelocity = velocity;
}

/**
 * Get the maximum angular velocity at which body can go to sleep.
 */
float PhysxBodyDesc::
get_sleep_angular_velocity() const {

  return _desc.sleepAngularVelocity;
}

/**
 * Set the number of solver iterations performed when processing
 * joint/contacts connected to this body.
 */
void PhysxBodyDesc::
set_solver_iteration_count(unsigned int count) {

  _desc.solverIterationCount = count;
}

/**
 * Get the number of solver iterations performed when processing
 * joint/contacts connected to this body.
 */
unsigned int PhysxBodyDesc::
get_solver_iteration_count() const {

  return _desc.solverIterationCount;
}

/**
 * Set the threshold for the energy-based sleeping algorithm.  Only used when
 * the BF_energy_sleep_test flag is set.
 */
void PhysxBodyDesc::
set_sleep_energy_threshold(float threshold) {

  _desc.sleepEnergyThreshold = threshold;
}

/**
 * Get the threshold for the energy-based sleeping algorithm.  Only used when
 * the BF_energy_sleep_test flag is set.
 */
float PhysxBodyDesc::
get_sleep_energy_threshold() const {

  return _desc.sleepEnergyThreshold;
}

/**
 * Set the damping factor for bodies that are about to sleep.
 */
void PhysxBodyDesc::
set_sleep_damping(float damping) {

  _desc.sleepDamping = damping;
}

/**
 * Get the damping factor for bodies that are about to sleep.
 */
float PhysxBodyDesc::
get_sleep_damping() const {

  return _desc.sleepDamping;
}

/**
 * Set the position and orientation of the center of mass.
 */
void PhysxBodyDesc::
set_mass_local_mat(const LMatrix4f &mat) {

  _desc.massLocalPose = PhysxManager::mat4_to_nxMat34(mat);
}

/**
 * Get the position and orientation of the center of mass.
 */
LMatrix4f PhysxBodyDesc::
get_mass_local_mat() const {

  return PhysxManager::nxMat34_to_mat4(_desc.massLocalPose);
}

/**
 * Set the diagonal mass space inertia tensor in bodies mass frame.
 */
void PhysxBodyDesc::
set_mass_space_inertia(const LVector3f &inertia) {

  _desc.massSpaceInertia = PhysxManager::vec3_to_nxVec3(inertia);
}

/**
 * Get the diagonal mass space inertia tensor in bodies mass frame.
 */
LVector3f PhysxBodyDesc::
get_mass_space_inertia() const {

  return PhysxManager::nxVec3_to_vec3(_desc.massSpaceInertia);
}

/**
 * Raise or lower individual body flags.
 */
void PhysxBodyDesc::
set_flag(const PhysxBodyFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

/**
 * Returns the specified body flag.
 */
bool PhysxBodyDesc::
get_flag(const PhysxBodyFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

/**
 * When CCD is globally enabled, it is still not performed if the motion
 * distance of all points on the body is below this threshold.
 */
void PhysxBodyDesc::
set_ccd_motion_threshold(float threshold) {

  _desc.CCDMotionThreshold = threshold;
}

/**
 *
 */
float PhysxBodyDesc::
get_ccd_motion_threshold() const {

  return _desc.CCDMotionThreshold;
}

/**
 * Set the body's initial wake up counter.
 */
void PhysxBodyDesc::
set_wake_up_counter(float value) {

  _desc.wakeUpCounter = value;
}

/**
 *
 */
float PhysxBodyDesc::
get_wake_up_counter() const {

  return _desc.wakeUpCounter;
}

/**
 * Set The force threshold for contact reports.
 */
void PhysxBodyDesc::
set_contact_report_threshold(float threshold) {

  _desc.contactReportThreshold = threshold;
}

/**
 *
 */
float PhysxBodyDesc::
get_contact_report_threshold() const {

  return _desc.contactReportThreshold;
}
