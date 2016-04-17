/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxD6JointDesc.cxx
 * @author enn0x
 * @date 2009-10-01
 */

#include "physxD6JointDesc.h"
#include "physxManager.h"
#include "physxJointDriveDesc.h"
#include "physxJointLimitSoftDesc.h"

/**
 *
 */
void PhysxD6JointDesc::
set_x_motion(PhysxD6JointMotion xMotion) {

  _desc.xMotion = (NxD6JointMotion)xMotion;
}

/**
 *
 */
void PhysxD6JointDesc::
set_y_motion(PhysxD6JointMotion yMotion) {

  _desc.yMotion = (NxD6JointMotion)yMotion;
}

/**
 *
 */
void PhysxD6JointDesc::
set_z_motion(PhysxD6JointMotion zMotion) {

  _desc.zMotion = (NxD6JointMotion)zMotion;
}

/**
 *
 */
void PhysxD6JointDesc::
set_swing1_motion(PhysxD6JointMotion swing1Motion) {

  _desc.swing1Motion = (NxD6JointMotion)swing1Motion;
}

/**
 *
 */
void PhysxD6JointDesc::
set_swing2_motion(PhysxD6JointMotion swing2Motion) {

  _desc.swing2Motion = (NxD6JointMotion)swing2Motion;
}

/**
 *
 */
void PhysxD6JointDesc::
set_twist_motion(PhysxD6JointMotion twistMotion) {

  _desc.twistMotion = (NxD6JointMotion)twistMotion;
}

/**
 *
 */
void PhysxD6JointDesc::
set_x_drive(const PhysxJointDriveDesc &drive) {

  _desc.xDrive = drive._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_y_drive(const PhysxJointDriveDesc &drive) {

  _desc.yDrive = drive._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_z_drive(const PhysxJointDriveDesc &drive) {

  _desc.zDrive = drive._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_swing_drive(const PhysxJointDriveDesc &drive) {

  _desc.swingDrive = drive._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_twist_drive(const PhysxJointDriveDesc &drive) {

  _desc.twistDrive = drive._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_slerp_drive(const PhysxJointDriveDesc &drive) {

  _desc.slerpDrive = drive._desc;
}

/**
 * Sets or clears a single D6JointFlag flag.
 */
void PhysxD6JointDesc::
set_flag(PhysxD6JointFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

/**
 *
 */
void PhysxD6JointDesc::
set_linear_limit(const PhysxJointLimitSoftDesc &limit) {

  _desc.linearLimit = limit._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_swing1_limit(const PhysxJointLimitSoftDesc &limit) {

  _desc.swing1Limit = limit._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_swing2_limit(const PhysxJointLimitSoftDesc &limit) {

  _desc.swing2Limit = limit._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_twist_limit_low(const PhysxJointLimitSoftDesc &limit) {

  _desc.twistLimit.low = limit._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_twist_limit_high(const PhysxJointLimitSoftDesc &limit) {

  _desc.twistLimit.high = limit._desc;
}

/**
 *
 */
void PhysxD6JointDesc::
set_projection_distance(float distance) {

  _desc.projectionDistance = distance;
}

/**
 *
 */
void PhysxD6JointDesc::
set_projection_angle(float angle) {

  _desc.projectionAngle = angle;
}

/**
 *
 */
void PhysxD6JointDesc::
set_gear_ratio(float ratio) {

  _desc.gearRatio = ratio;
}

/**
 *
 */
void PhysxD6JointDesc::
set_drive_position(const LPoint3f &pos) {

  nassertv(!pos.is_nan());
  _desc.drivePosition = PhysxManager::point3_to_nxVec3(pos);
}

/**
 *
 */
void PhysxD6JointDesc::
set_drive_linear_velocity(const LVector3f &v) {

  nassertv(!v.is_nan());
  _desc.driveLinearVelocity = PhysxManager::vec3_to_nxVec3(v);
}

/**
 *
 */
void PhysxD6JointDesc::
set_drive_angular_velocity(const LVector3f &v) {

  nassertv(!v.is_nan());
  _desc.driveAngularVelocity = PhysxManager::vec3_to_nxVec3(v);
}

/**
 *
 */
void PhysxD6JointDesc::
set_drive_orientation(const LQuaternionf &quat) {

  _desc.driveOrientation = PhysxManager::quat_to_nxQuat(quat);
}

/**
 * Use this to enable joint projection.  Default is PM_none.
 */
void PhysxD6JointDesc::
set_projection_mode(PhysxProjectionMode mode) {

  _desc.projectionMode = (NxJointProjectionMode)mode;
}

/**
 *
 */
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_x_motion() const {

  return (PhysxD6JointMotion)_desc.xMotion;
}

/**
 *
 */
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_y_motion() const {

  return (PhysxD6JointMotion)_desc.yMotion;
}

/**
 *
 */
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_z_motion() const {

  return (PhysxD6JointMotion)_desc.zMotion;
}

/**
 *
 */
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_swing1_motion() const {

  return (PhysxD6JointMotion)_desc.swing1Motion;
}

/**
 *
 */
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_swing2_motion() const {

  return (PhysxD6JointMotion)_desc.swing2Motion;
}

/**
 *
 */
PhysxEnums::PhysxD6JointMotion PhysxD6JointDesc::
get_twist_motion() const {

  return (PhysxD6JointMotion)_desc.twistMotion;
}

/**
 *
 */
PhysxJointDriveDesc PhysxD6JointDesc::
get_x_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.xDrive;
  return value;
}

/**
 *
 */
PhysxJointDriveDesc PhysxD6JointDesc::
get_y_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.yDrive;
  return value;
}

/**
 *
 */
PhysxJointDriveDesc PhysxD6JointDesc::
get_z_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.zDrive;
  return value;
}

/**
 *
 */
PhysxJointDriveDesc PhysxD6JointDesc::
get_swing_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.swingDrive;
  return value;
}

/**
 *
 */
PhysxJointDriveDesc PhysxD6JointDesc::
get_twist_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.twistDrive;
  return value;
}

/**
 *
 */
PhysxJointDriveDesc PhysxD6JointDesc::
get_slerp_drive() const {

  PhysxJointDriveDesc value;
  value._desc = _desc.slerpDrive;
  return value;
}

/**
 *
 */
bool PhysxD6JointDesc::
get_flag(PhysxD6JointFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

/**
 *
 */
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_linear_limit() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.linearLimit;
  return value;
}

/**
 *
 */
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_swing1_limit() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.swing1Limit;
  return value;
}

/**
 *
 */
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_swing2_limit() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.swing2Limit;
  return value;
}

/**
 *
 */
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_twist_limit_low() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.twistLimit.low;
  return value;
}

/**
 *
 */
PhysxJointLimitSoftDesc PhysxD6JointDesc::
get_twist_limit_high() const {

  PhysxJointLimitSoftDesc value;
  value._desc = _desc.twistLimit.high;
  return value;
}

/**
 *
 */
float PhysxD6JointDesc::
get_projection_distance() const {

  return _desc.projectionDistance;
}

/**
 *
 */
float PhysxD6JointDesc::
get_projection_angle() const {

  return _desc.projectionAngle;
}

/**
 *
 */
float PhysxD6JointDesc::
get_gear_ratio() const {

  return _desc.gearRatio;
}

/**
 *
 */
LPoint3f PhysxD6JointDesc::
get_drive_position() const {

  return PhysxManager::nxVec3_to_point3(_desc.drivePosition);
}

/**
 *
 */
LVector3f PhysxD6JointDesc::
get_drive_linear_velocity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.driveLinearVelocity);
}

/**
 *
 */
LVector3f PhysxD6JointDesc::
get_drive_angular_velocity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.driveAngularVelocity);
}

/**
 *
 */
LQuaternionf PhysxD6JointDesc::
get_drive_orientation() const {

  return PhysxManager::nxQuat_to_quat(_desc.driveOrientation);
}

/**
 *
 */
PhysxEnums::PhysxProjectionMode PhysxD6JointDesc::
get_projection_mode() const {

  return (PhysxProjectionMode)_desc.projectionMode;
}
