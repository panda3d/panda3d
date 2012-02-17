// Filename: bulletWheel.h
// Created by:  enn0x (17Feb10)
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

#ifndef __BULLET_WHEEL_H__
#define __BULLET_WHEEL_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"
#include "pandaNode.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletWheelRaycastInfo
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletWheelRaycastInfo {

PUBLISHED:
  INLINE ~BulletWheelRaycastInfo();

  INLINE bool is_in_contact() const;
  INLINE PN_stdfloat get_suspension_length() const;
  INLINE LVector3 get_contact_normal_ws() const;
  INLINE LVector3 get_wheel_direction_ws() const;
  INLINE LVector3 get_wheel_axle_ws() const;
  INLINE LPoint3 get_contact_point_ws() const;
  INLINE LPoint3 get_hard_point_ws() const;
  INLINE PandaNode *get_ground_object() const;

public:
  BulletWheelRaycastInfo(btWheelInfo::RaycastInfo &info);

private:
  btWheelInfo::RaycastInfo &_info;
};

////////////////////////////////////////////////////////////////////
//       Class : BulletWheel
// Description : One wheel of a BulletVehicle. Instances should not
//               be created directly but using the factory method
//               BulletVehicle::create_wheel().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletWheel {

PUBLISHED:
  INLINE ~BulletWheel();

  void set_suspension_stiffness(PN_stdfloat value);
  void set_max_suspension_travel_cm(PN_stdfloat value);
  void set_friction_slip(PN_stdfloat value);
  void set_max_suspension_force(PN_stdfloat value);
  void set_wheels_damping_compression(PN_stdfloat value);
  void set_wheels_damping_relaxation(PN_stdfloat value);
  void set_roll_influence(PN_stdfloat value);
  void set_wheel_radius(PN_stdfloat value);
  void set_steering(PN_stdfloat value);
  void set_rotation(PN_stdfloat value);
  void set_delta_rotation(PN_stdfloat value);
  void set_engine_force(PN_stdfloat value);
  void set_brake(PN_stdfloat value);
  void set_skid_info(PN_stdfloat value);
  void set_wheels_suspension_force(PN_stdfloat value);
  void set_suspension_relative_velocity(PN_stdfloat value);
  void set_clipped_inv_connection_point_cs(PN_stdfloat value);
  void set_chassis_connection_point_cs(const LPoint3 &pos);
  void set_wheel_direction_cs(const LVector3 &dir);
  void set_wheel_axle_cs(const LVector3 &axle);
  void set_world_transform(const LMatrix4 &mat);
  void set_front_wheel(bool value);
  void set_node(PandaNode *node);

  PN_stdfloat get_suspension_rest_length() const;
  PN_stdfloat get_suspension_stiffness() const;
  PN_stdfloat get_max_suspension_travel_cm() const;
  PN_stdfloat get_friction_slip() const;
  PN_stdfloat get_max_suspension_force() const;
  PN_stdfloat get_wheels_damping_compression() const;
  PN_stdfloat get_wheels_damping_relaxation() const;
  PN_stdfloat get_roll_influence() const;
  PN_stdfloat get_wheel_radius() const;
  PN_stdfloat get_steering() const;
  PN_stdfloat get_rotation() const;
  PN_stdfloat get_delta_rotation() const;
  PN_stdfloat get_engine_force() const;
  PN_stdfloat get_brake() const;
  PN_stdfloat get_skid_info() const;
  PN_stdfloat get_wheels_suspension_force() const;
  PN_stdfloat get_suspension_relative_velocity() const;
  PN_stdfloat get_clipped_inv_connection_point_cs() const;
  LPoint3 get_chassis_connection_point_cs() const;
  LVector3 get_wheel_direction_cs() const;
  LVector3 get_wheel_axle_cs() const;
  LMatrix4 get_world_transform() const;
  bool is_front_wheel() const;
  PandaNode *get_node() const;
  BulletWheelRaycastInfo get_raycast_info() const;

public:
  BulletWheel(btWheelInfo &info);

  INLINE static BulletWheel empty();

private:
  btWheelInfo &_info;
};

#include "bulletWheel.I"

#endif // __BULLET_WHEEL_H__
