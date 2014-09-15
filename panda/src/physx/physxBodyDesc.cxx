// Filename: physxBodyDesc.cxx
// Created by:  enn0x (05Sep09)
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

#include "physxBodyDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_mass
//       Access: Published
//  Description: Set the mass of body. 
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_mass(float mass) {

  _desc.mass = mass;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_mass
//       Access: Published
//  Description: Get the mass of body. 
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_mass() const {

  return _desc.mass;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_linear_damping
//       Access: Published
//  Description: Set the linear damping applied to the body.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_linear_damping(float damping) {

  _desc.linearDamping = damping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_linear_damping
//       Access: Published
//  Description: Get the linear damping applied to the body.
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_linear_damping() const {

  return _desc.linearDamping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_angular_damping
//       Access: Published
//  Description: Set the angular damping applied to the body.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_angular_damping(float damping) {

  _desc.angularDamping = damping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_angular_damping
//       Access: Published
//  Description: Get the angular damping applied to the body.
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_angular_damping() const {

  return _desc.angularDamping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_linear_velocity
//       Access: Published
//  Description: Set the linear Velocity of the body.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_linear_velocity(const LVector3f &velocity) {

  _desc.linearVelocity = PhysxManager::vec3_to_nxVec3(velocity);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_linear_velocity
//       Access: Published
//  Description: Get the linear Velocity of the body.
////////////////////////////////////////////////////////////////////
LVector3f PhysxBodyDesc::
get_linear_velocity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.linearVelocity);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_angular_velocity
//       Access: Published
//  Description: Set the angular velocity of the body.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_angular_velocity(const LVector3f &velocity) {

  _desc.angularVelocity = PhysxManager::vec3_to_nxVec3(velocity);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_angular_velocity
//       Access: Published
//  Description: Get the angular velocity of the body.
////////////////////////////////////////////////////////////////////
LVector3f PhysxBodyDesc::
get_angular_velocity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.angularVelocity);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_max_angular_velocity
//       Access: Published
//  Description: Set the maximum allowed angular velocity for this
//               body.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_max_angular_velocity(float maximum) {

  _desc.maxAngularVelocity = maximum;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_max_angular_velocity
//       Access: Published
//  Description: Get the maximum allowed angular velocity for this
//               body.
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_max_angular_velocity() const {

  return _desc.maxAngularVelocity;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_sleep_linear_velocity
//       Access: Published
//  Description: Set the maximum linear velocity at which the body
//               can go to sleep.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_sleep_linear_velocity(float velocity) {

  _desc.sleepLinearVelocity = velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_sleep_linear_velocity
//       Access: Published
//  Description: Get the maximum linear velocity at which the body
//               can go to sleep.
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_sleep_linear_velocity() const {

  return _desc.sleepLinearVelocity;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_sleep_angular_velocity
//       Access: Published
//  Description: Set the maximum angular velocity at which body
//               can go to sleep.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_sleep_angular_velocity(float velocity) {

  _desc.sleepAngularVelocity = velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_sleep_angular_velocity
//       Access: Published
//  Description: Get the maximum angular velocity at which body
//               can go to sleep.
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_sleep_angular_velocity() const {

  return _desc.sleepAngularVelocity;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_solver_iteration_count
//       Access: Published
//  Description: Set the number of solver iterations performed
//               when processing joint/contacts connected to this
//               body.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_solver_iteration_count(unsigned int count) {

  _desc.solverIterationCount = count;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_solver_iteration_count
//       Access: Published
//  Description: Get the number of solver iterations performed
//               when processing joint/contacts connected to this
//               body.
////////////////////////////////////////////////////////////////////
unsigned int PhysxBodyDesc::
get_solver_iteration_count() const {

  return _desc.solverIterationCount;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_sleep_energy_threshold
//       Access: Published
//  Description: Set the threshold for the energy-based sleeping
//               algorithm. Only used when the BF_energy_sleep_test
//               flag is set.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_sleep_energy_threshold(float threshold) {

  _desc.sleepEnergyThreshold = threshold;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_sleep_energy_threshold
//       Access: Published
//  Description: Get the threshold for the energy-based sleeping
//               algorithm. Only used when the BF_energy_sleep_test
//               flag is set.
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_sleep_energy_threshold() const {

  return _desc.sleepEnergyThreshold;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_sleep_damping
//       Access: Published
//  Description: Set the damping factor for bodies that are about
//               to sleep.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_sleep_damping(float damping) {

  _desc.sleepDamping = damping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_sleep_damping
//       Access: Published
//  Description: Get the damping factor for bodies that are about
//               to sleep.
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_sleep_damping() const {

  return _desc.sleepDamping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_mass_local_mat
//       Access: Published
//  Description: Set the position and orientation of the center
//               of mass.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_mass_local_mat(const LMatrix4f &mat) {

  _desc.massLocalPose = PhysxManager::mat4_to_nxMat34(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_mass_local_mat
//       Access: Published
//  Description: Get the position and orientation of the center
//               of mass.
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxBodyDesc::
get_mass_local_mat() const {

  return PhysxManager::nxMat34_to_mat4(_desc.massLocalPose);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_mass_space_inertia
//       Access: Published
//  Description: Set the diagonal mass space inertia tensor in
//               bodies mass frame.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_mass_space_inertia(const LVector3f &inertia) {

  _desc.massSpaceInertia = PhysxManager::vec3_to_nxVec3(inertia);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_mass_space_inertia
//       Access: Published
//  Description: Get the diagonal mass space inertia tensor in
//               bodies mass frame.
////////////////////////////////////////////////////////////////////
LVector3f PhysxBodyDesc::
get_mass_space_inertia() const {

  return PhysxManager::nxVec3_to_vec3(_desc.massSpaceInertia);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_flag
//       Access: Published
//  Description: Raise or lower individual body flags.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_flag(const PhysxBodyFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_flag
//       Access: Published
//  Description: Returns the specified body flag.
////////////////////////////////////////////////////////////////////
bool PhysxBodyDesc::
get_flag(const PhysxBodyFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_ccd_motion_threshold
//       Access: Published
//  Description: When CCD is globally enabled, it is still not
//               performed if the motion distance of all points on
//               the body is below this threshold. 
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_ccd_motion_threshold(float threshold) {

  _desc.CCDMotionThreshold = threshold;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_ccd_motion_threshold
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_ccd_motion_threshold() const {

  return _desc.CCDMotionThreshold;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_wake_up_counter
//       Access: Published
//  Description: Set the body's initial wake up counter.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_wake_up_counter(float value) {

  _desc.wakeUpCounter = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_wake_up_counter
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_wake_up_counter() const {

  return _desc.wakeUpCounter;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::set_contact_report_threshold
//       Access: Published
//  Description: Set The force threshold for contact reports.
////////////////////////////////////////////////////////////////////
void PhysxBodyDesc::
set_contact_report_threshold(float threshold) {

  _desc.contactReportThreshold = threshold;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBodyDesc::get_contact_report_threshold
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxBodyDesc::
get_contact_report_threshold() const {

  return _desc.contactReportThreshold;
}

