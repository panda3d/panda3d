// Filename: physxD6JointDesc.h
// Created by:  pratt (Jun 20, 2006)
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

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxJointDesc.h"

class PhysxJointDriveDesc;
class PhysxJointLimitSoftDesc;
class PhysxJointLimitSoftPairDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxD6JointDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxD6JointDesc : public PhysxJointDesc {
PUBLISHED:
  PhysxD6JointDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  LVecBase3f get_drive_angular_velocity() const;
  LVecBase3f get_drive_linear_velocity() const;
  LQuaternionf get_drive_orientation() const;
  LVecBase3f get_drive_position() const;
  INLINE unsigned int get_flags() const;
  INLINE float get_gear_ratio() const;
  PhysxJointLimitSoftDesc & get_linear_limit() const;
  INLINE float get_projection_angle() const;
  INLINE float get_projection_distance() const;
  PhysxJointProjectionMode get_projection_mode() const;
  PhysxJointDriveDesc & get_slerp_drive() const;
  PhysxJointLimitSoftDesc & get_swing1_limit() const;
  PhysxD6JointMotion get_swing1_motion() const;
  PhysxJointLimitSoftDesc & get_swing2_limit() const;
  PhysxD6JointMotion get_swing2_motion() const;
  PhysxJointDriveDesc & get_swing_drive() const;
  PhysxJointDriveDesc & get_twist_drive() const;
  PhysxJointLimitSoftPairDesc & get_twist_limit() const;
  PhysxD6JointMotion get_twist_motion() const;
  PhysxJointDriveDesc & get_x_drive() const;
  PhysxD6JointMotion get_x_motion() const;
  PhysxJointDriveDesc & get_y_drive() const;
  PhysxD6JointMotion get_y_motion() const;
  PhysxJointDriveDesc & get_z_drive() const;
  PhysxD6JointMotion get_z_motion() const;

  void set_drive_angular_velocity( LVecBase3f value );
  void set_drive_linear_velocity( LVecBase3f value );
  void set_drive_orientation( LQuaternionf value );
  void set_drive_position( LVecBase3f value );
  INLINE void set_flags( unsigned int value );
  INLINE void set_gear_ratio( float value );
  void set_linear_limit( PhysxJointLimitSoftDesc & value );
  INLINE void set_projection_angle( float value );
  INLINE void set_projection_distance( float value );
  void set_projection_mode( PhysxJointProjectionMode value );
  void set_slerp_drive( PhysxJointDriveDesc & value );
  void set_swing1_limit( PhysxJointLimitSoftDesc & value );
  void set_swing1_motion( PhysxD6JointMotion value );
  void set_swing2_limit( PhysxJointLimitSoftDesc & value );
  void set_swing2_motion( PhysxD6JointMotion value );
  void set_swing_drive( PhysxJointDriveDesc & value );
  void set_twist_drive( PhysxJointDriveDesc & value );
  void set_twist_limit( PhysxJointLimitSoftPairDesc & value );
  void set_twist_motion( PhysxD6JointMotion value );
  void set_x_drive( PhysxJointDriveDesc & value );
  void set_x_motion( PhysxD6JointMotion value );
  void set_y_drive( PhysxJointDriveDesc & value );
  void set_y_motion( PhysxD6JointMotion value );
  void set_z_drive( PhysxJointDriveDesc & value );
  void set_z_motion( PhysxD6JointMotion value );

public:
  NxD6JointDesc nD6JointDesc;
};

#include "physxD6JointDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXD6JOINTDESC_H
