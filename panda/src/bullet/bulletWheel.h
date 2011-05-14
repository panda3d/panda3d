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

#include "lvector3.h"
#include "lpoint3.h"
#include "lmatrix.h"
#include "pandaNode.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletWheel
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletWheel {

PUBLISHED:
  INLINE ~BulletWheel();

  void set_suspension_stiffness(float value);
  void set_max_suspension_travel_cm(float value);
  void set_friction_slip(float value);
  void set_max_suspension_force(float value);
  void set_wheels_damping_compression(float value);
  void set_wheels_damping_relaxation(float value);
  void set_roll_influence(float value);
  void set_wheel_radius(float value);
  void set_steering(float value);
  void set_rotation(float value);
  void set_delta_rotation(float value);
  void set_engine_force(float value);
  void set_brake(float value);
  void set_skid_info(float value);
  void set_wheels_suspension_force(float value);
  void set_suspension_relative_velocity(float value);
  void set_clipped_inv_connection_point_cs(float value);
  void set_chassis_connection_point_cs(const LPoint3f &pos);
  void set_wheel_direction_cs(const LVector3f &dir);
  void set_wheel_axle_cs(const LVector3f &axle);
  void set_world_transform(const LMatrix4f &mat);
  void set_front_wheel(bool value);
  void set_node(PandaNode *node);

  float get_suspension_rest_length() const;
  float get_suspension_stiffness() const;
  float get_max_suspension_travel_cm() const;
  float get_friction_slip() const;
  float get_max_suspension_force() const;
  float get_wheels_damping_compression() const;
  float get_wheels_damping_relaxation() const;
  float get_roll_influence() const;
  float get_wheel_radius() const;
  float get_steering() const;
  float get_rotation() const;
  float get_delta_rotation() const;
  float get_engine_force() const;
  float get_brake() const;
  float get_skid_info() const;
  float get_wheels_suspension_force() const;
  float get_suspension_relative_velocity() const;
  float get_clipped_inv_connection_point_cs() const;
  LPoint3f get_chassis_connection_point_cs() const;
  LVector3f get_wheel_direction_cs() const;
  LVector3f get_wheel_axle_cs() const;
  LMatrix4f get_world_transform() const;
  bool is_front_wheel() const;
  PandaNode *get_node() const;

public:
  BulletWheel(btWheelInfo &info);

  INLINE static BulletWheel empty();

private:
  btWheelInfo &_info;
};

#include "bulletWheel.I"

#endif // __BULLET_WHEEL_H__
