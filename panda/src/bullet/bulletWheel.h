/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletWheel.h
 * @author enn0x
 * @date 2010-02-17
 */

#ifndef __BULLET_WHEEL_H__
#define __BULLET_WHEEL_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"
#include "pandaNode.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletWheelRaycastInfo {

PUBLISHED:
  INLINE ~BulletWheelRaycastInfo();

  bool is_in_contact() const;
  PN_stdfloat get_suspension_length() const;
  LVector3 get_contact_normal_ws() const;
  LVector3 get_wheel_direction_ws() const;
  LVector3 get_wheel_axle_ws() const;
  LPoint3 get_contact_point_ws() const;
  LPoint3 get_hard_point_ws() const;
  PandaNode *get_ground_object() const;

  MAKE_PROPERTY(in_contact, is_in_contact);
  MAKE_PROPERTY(suspension_length, get_suspension_length);
  MAKE_PROPERTY(contact_normal_ws, get_contact_normal_ws);
  MAKE_PROPERTY(wheel_direction_ws, get_wheel_direction_ws);
  MAKE_PROPERTY(wheel_axle_ws, get_wheel_axle_ws);
  MAKE_PROPERTY(contact_point_ws, get_contact_point_ws);
  MAKE_PROPERTY(hard_point_ws, get_hard_point_ws);
  MAKE_PROPERTY(ground_object, get_ground_object);

public:
  BulletWheelRaycastInfo(btWheelInfo::RaycastInfo &info);

private:
  btWheelInfo::RaycastInfo &_info;
};

/**
 * One wheel of a BulletVehicle.  Instances should not be created directly but
 * using the factory method BulletVehicle::create_wheel().
 */
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

  MAKE_PROPERTY(raycast_info, get_raycast_info);
  MAKE_PROPERTY(suspension_rest_length, get_suspension_rest_length);
  MAKE_PROPERTY(suspension_stiffness, get_suspension_stiffness, set_suspension_stiffness);
  MAKE_PROPERTY(max_suspension_travel_cm, get_max_suspension_travel_cm, set_max_suspension_travel_cm);
  MAKE_PROPERTY(friction_slip, get_friction_slip, set_friction_slip);
  MAKE_PROPERTY(max_suspension_force, get_max_suspension_force, set_max_suspension_force);
  MAKE_PROPERTY(wheels_damping_compression, get_wheels_damping_compression, set_wheels_damping_compression);
  MAKE_PROPERTY(wheels_damping_relaxation, get_wheels_damping_relaxation, set_wheels_damping_relaxation);
  MAKE_PROPERTY(roll_influence, get_roll_influence, set_roll_influence);
  MAKE_PROPERTY(wheel_radius, get_wheel_radius, set_wheel_radius);
  MAKE_PROPERTY(steering, get_steering, set_steering);
  MAKE_PROPERTY(rotation, get_rotation, set_rotation);
  MAKE_PROPERTY(delta_rotation, get_delta_rotation, set_delta_rotation);
  MAKE_PROPERTY(engine_force, get_engine_force, set_engine_force);
  MAKE_PROPERTY(brake, get_brake, set_brake);
  MAKE_PROPERTY(skid_info, get_skid_info, set_skid_info);
  MAKE_PROPERTY(wheels_suspension_force, get_wheels_suspension_force, set_wheels_suspension_force);
  MAKE_PROPERTY(suspension_relative_velocity, get_suspension_relative_velocity, set_suspension_relative_velocity);
  MAKE_PROPERTY(clipped_inv_connection_point_cs, get_clipped_inv_connection_point_cs, set_clipped_inv_connection_point_cs);
  MAKE_PROPERTY(chassis_connection_point_cs, get_chassis_connection_point_cs, set_chassis_connection_point_cs);
  MAKE_PROPERTY(wheel_direction_cs, get_wheel_direction_cs, set_wheel_direction_cs);
  MAKE_PROPERTY(wheel_axle_cs, get_wheel_axle_cs, set_wheel_axle_cs);
  MAKE_PROPERTY(world_transform, get_world_transform, set_world_transform);
  MAKE_PROPERTY(front_wheel, is_front_wheel, set_front_wheel);
  MAKE_PROPERTY(node, get_node, set_node);

public:
  BulletWheel(btWheelInfo &info);

  INLINE static BulletWheel empty();

private:
  btWheelInfo &_info;
};

#include "bulletWheel.I"

#endif // __BULLET_WHEEL_H__
