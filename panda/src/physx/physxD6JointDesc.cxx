// Filename: physxD6JointDesc.cxx
// Created by:  enn0x (01Octp09)
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

#include "physxD6JointDesc.h"
#include "physxManager.h"
#include "physxJointDriveDesc.h"
#include "physxJointLimitSoftDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_x_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_x_motion(PhysxD6JointMotion xMotion) {

  _desc.xMotion = (NxD6JointMotion)xMotion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_y_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_y_motion(PhysxD6JointMotion yMotion) {

  _desc.yMotion = (NxD6JointMotion)yMotion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_z_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_z_motion(PhysxD6JointMotion zMotion) {

  _desc.zMotion = (NxD6JointMotion)zMotion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_swing1_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_swing1_motion(PhysxD6JointMotion swing1Motion) {

  _desc.swing1Motion = (NxD6JointMotion)swing1Motion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_swing2_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_swing2_motion(PhysxD6JointMotion swing2Motion) {

  _desc.swing2Motion = (NxD6JointMotion)swing2Motion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_twist_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_twist_motion(PhysxD6JointMotion twistMotion) {

  _desc.twistMotion = (NxD6JointMotion)twistMotion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_x_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_x_drive(const PhysxJointDriveDesc &drive) {

  _desc.xDrive = drive._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_y_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_y_drive(const PhysxJointDriveDesc &drive) {

  _desc.yDrive = drive._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_z_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_z_drive(const PhysxJointDriveDesc &drive) {

  _desc.zDrive = drive._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_swing_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_swing_drive(const PhysxJointDriveDesc &drive) {

  _desc.swingDrive = drive._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_twist_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_twist_drive(const PhysxJointDriveDesc &drive) {

  _desc.twistDrive = drive._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_slerp_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_slerp_drive(const PhysxJointDriveDesc &drive) {

  _desc.slerpDrive = drive._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_flag
//       Access: Published
//  Description: Sets or clears a single D6JointFlag flag.
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_flag(PhysxD6JointFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_linear_limit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_linear_limit(const PhysxJointLimitSoftDesc &limit) {

  _desc.linearLimit = limit._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_swing1_limit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_swing1_limit(const PhysxJointLimitSoftDesc &limit) {

  _desc.swing1Limit = limit._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_swing2_limit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_swing2_limit(const PhysxJointLimitSoftDesc &limit) {

  _desc.swing2Limit = limit._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_twist_limit_low
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_twist_limit_low(const PhysxJointLimitSoftDesc &limit) {

  _desc.twistLimit.low = limit._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_twist_limit_high
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_twist_limit_high(const PhysxJointLimitSoftDesc &limit) {

  _desc.twistLimit.high = limit._desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_projection_distance
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_projection_distance(float distance) {

  _desc.projectionDistance = distance;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_projection_angle
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_projection_angle(float angle) {

  _desc.projectionAngle = angle;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_gear_ratio
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_gear_ratio(float ratio) {

  _desc.gearRatio = ratio;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_drive_position
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_drive_position(const LPoint3f &pos) {

  nassertv(!pos.is_nan());
  _desc.drivePosition = PhysxManager::point3_to_nxVec3(pos);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_drive_linear_velocity
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_drive_linear_velocity(const LVector3f &v) {

  nassertv(!v.is_nan());
  _desc.driveLinearVelocity = PhysxManager::vec3_to_nxVec3(v);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_drive_angular_velocity
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_drive_angular_velocity(const LVector3f &v) {

  nassertv(!v.is_nan());
  _desc.driveAngularVelocity = PhysxManager::vec3_to_nxVec3(v);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_drive_orientation
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_drive_orientation(const LQuaternionf &quat) {

  _desc.driveOrientation = PhysxManager::quat_to_nxQuat(quat);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::set_projection_mode
//       Access: Published
//  Description: Use this to enable joint projection.
//               Default is PM_none.
////////////////////////////////////////////////////////////////////
void PhysxD6JointDesc::
set_projection_mode(PhysxProjectionMode mode) {

  _desc.projectionMode = (NxJointProjectionMode)mode;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_x_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_x_motion() const {

  return (PhysxD6JointMotion)_desc.xMotion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_y_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_y_motion() const {

  return (PhysxD6JointMotion)_desc.yMotion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_z_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_z_motion() const {

  return (PhysxD6JointMotion)_desc.zMotion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_swing1_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_swing1_motion() const {

  return (PhysxD6JointMotion)_desc.swing1Motion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_swing2_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_swing2_motion() const {

  return (PhysxD6JointMotion)_desc.swing2Motion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_twist_motion
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_twist_motion() const {

  return (PhysxD6JointMotion)_desc.twistMotion;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_x_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointDriveDesc PhysxD6JointDesc::
get_x_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.xDrive;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_y_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointDriveDesc PhysxD6JointDesc::
get_y_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.yDrive;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_z_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointDriveDesc PhysxD6JointDesc::
get_z_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.zDrive;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_swing_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointDriveDesc PhysxD6JointDesc::
get_swing_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.swingDrive;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_twist_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointDriveDesc PhysxD6JointDesc::
get_twist_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.twistDrive;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_slerp_drive
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointDriveDesc PhysxD6JointDesc::
get_slerp_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.slerpDrive;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_flag
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxD6JointDesc::
get_flag(PhysxD6JointFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_linear_limit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_linear_limit() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.linearLimit;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_swing1_limit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_swing1_limit() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.swing1Limit;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_swing2_limit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_swing2_limit() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.swing2Limit;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_twist_limit_low
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_twist_limit_low() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.twistLimit.low;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_twist_limit_high
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_twist_limit_high() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.twistLimit.high;
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_projection_distance
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxD6JointDesc::
get_projection_distance() const {

  return _desc.projectionDistance;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_projection_angle
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxD6JointDesc::
get_projection_angle() const {

  return _desc.projectionAngle;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_gear_ratio
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxD6JointDesc::
get_gear_ratio() const {

  return _desc.gearRatio;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_drive_position
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3f PhysxD6JointDesc::
get_drive_position() const {

  return PhysxManager::nxVec3_to_point3(_desc.drivePosition);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_drive_linear_velocity
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LVector3f PhysxD6JointDesc::
get_drive_linear_velocity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.driveLinearVelocity);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_drive_angular_velocity
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LVector3f PhysxD6JointDesc::
get_drive_angular_velocity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.driveAngularVelocity);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_drive_orientation
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LQuaternionf PhysxD6JointDesc::
get_drive_orientation() const {

  return PhysxManager::nxQuat_to_quat(_desc.driveOrientation);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxD6JointDesc::get_projection_mode
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxProjectionMode PhysxD6JointDesc::
get_projection_mode() const {

  return (PhysxProjectionMode)_desc.projectionMode;
}

