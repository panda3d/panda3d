// Filename: physxD6JointDesc.h
// Created by:  enn0x (01Oct09)
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

#ifndef PHYSXD6JOINTDESC_H
#define PHYSXD6JOINTDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxJointDesc.h"
#include "physx_includes.h"

class PhysxJointDriveDesc;
class PhysxJointLimitSoftDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxD6JointDesc
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxD6JointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxD6JointDesc();
  INLINE ~PhysxD6JointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_flag(PhysxD6JointFlag flag, bool value);
  void set_projection_distance(float distance);
  void set_projection_angle(float angle);
  void set_gear_ratio(float ratio);
  void set_drive_position(const LPoint3f &pos);
  void set_drive_linear_velocity(const LVector3f &v);
  void set_drive_angular_velocity(const LVector3f &v);
  void set_drive_orientation(const LQuaternionf &quat);
  void set_projection_mode(PhysxProjectionMode mode);
  void set_x_motion(PhysxD6JointMotion xMotion);
  void set_y_motion(PhysxD6JointMotion yMotion);
  void set_z_motion(PhysxD6JointMotion zMotion);
  void set_swing1_motion(PhysxD6JointMotion xMotion);
  void set_swing2_motion(PhysxD6JointMotion yMotion);
  void set_twist_motion(PhysxD6JointMotion zMotion);
  void set_x_drive(const PhysxJointDriveDesc &drive);
  void set_y_drive(const PhysxJointDriveDesc &drive);
  void set_z_drive(const PhysxJointDriveDesc &drive);
  void set_swing_drive(const PhysxJointDriveDesc &drive);
  void set_twist_drive(const PhysxJointDriveDesc &drive);
  void set_slerp_drive(const PhysxJointDriveDesc &drive);
  void set_linear_limit(const PhysxJointLimitSoftDesc &limit);
  void set_swing1_limit(const PhysxJointLimitSoftDesc &limit);
  void set_swing2_limit(const PhysxJointLimitSoftDesc &limit);
  void set_twist_limit_low(const PhysxJointLimitSoftDesc &limit);
  void set_twist_limit_high(const PhysxJointLimitSoftDesc &limit);

  bool get_flag(PhysxD6JointFlag flag) const;
  float get_projection_distance() const;
  float get_projection_angle() const;
  float get_gear_ratio() const;
  LPoint3f get_drive_position() const;
  LVector3f get_drive_linear_velocity() const;
  LVector3f get_drive_angular_velocity() const;
  LQuaternionf get_drive_orientation() const;
  PhysxProjectionMode get_projection_mode() const;
  PhysxD6JointMotion get_x_motion() const;
  PhysxD6JointMotion get_y_motion() const;
  PhysxD6JointMotion get_z_motion() const;
  PhysxD6JointMotion get_swing1_motion() const;
  PhysxD6JointMotion get_swing2_motion() const;
  PhysxD6JointMotion get_twist_motion() const;
  PhysxJointDriveDesc get_x_drive() const;
  PhysxJointDriveDesc get_y_drive() const;
  PhysxJointDriveDesc get_z_drive() const;
  PhysxJointDriveDesc get_swing_drive() const;
  PhysxJointDriveDesc get_twist_drive() const;
  PhysxJointDriveDesc get_slerp_drive() const;
  PhysxJointLimitSoftDesc get_linear_limit() const;
  PhysxJointLimitSoftDesc get_swing1_limit() const;
  PhysxJointLimitSoftDesc get_swing2_limit() const;
  PhysxJointLimitSoftDesc get_twist_limit_low() const;
  PhysxJointLimitSoftDesc get_twist_limit_high() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };
  NxD6JointDesc _desc;
};

#include "physxD6JointDesc.I"

#endif // PHYSXD6JOINTDESC_H
