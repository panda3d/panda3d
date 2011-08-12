// Filename: bulletVehicle.h
// Created by:  enn0x (16Feb10)
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

#ifndef __BULLET_VEHICLE_H__
#define __BULLET_VEHICLE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "typedReferenceCount.h"
#include "lpoint3.h"
#include "lvector3.h"

class BulletWorld;
class BulletRigidBodyNode;
class BulletWheel;

////////////////////////////////////////////////////////////////////
//       Class : BulletVehicleTuning
// Description : 
////////////////////////////////////////////////////////////////////
class BulletVehicleTuning {

PUBLISHED:
  INLINE void set_suspension_stiffness(float value);
  INLINE void set_suspension_compression(float value);
  INLINE void set_suspension_damping(float value);
  INLINE void set_max_suspension_travel_cm(float value);
  INLINE void set_friction_slip(float value);
  INLINE void set_max_suspension_force(float value);

  INLINE float get_suspension_stiffness() const;
  INLINE float get_suspension_compression() const;
  INLINE float get_suspension_damping() const;
  INLINE float get_max_suspension_travel_cm() const;
  INLINE float get_friction_slip() const;
  INLINE float get_max_suspension_force() const;

private:
  btRaycastVehicle::btVehicleTuning _;

  friend class BulletVehicle;
};

////////////////////////////////////////////////////////////////////
//       Class : BulletVehicle
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletVehicle : public TypedReferenceCount {

PUBLISHED:
  BulletVehicle(BulletWorld *world, BulletRigidBodyNode *chassis);
  INLINE ~BulletVehicle();

  void set_coordinate_system(BulletUpAxis up);
  void set_steering_value(float steering, int idx);
  void set_brake(float brake, int idx);
  void set_pitch_control(float pitch);

  BulletRigidBodyNode *get_chassis();
  float get_current_speed_km_hour() const;
  float get_steering_value(int idx) const;
  LVector3f get_forward_vector() const;

  void reset_suspension();
  void apply_engine_force(float force, int idx);

  // Wheels
  BulletWheel create_wheel();

  INLINE int get_num_wheels() const;
  BulletWheel get_wheel(int idx) const;
  MAKE_SEQ(get_wheels, get_num_wheels, get_wheel);

  // Tuning
  INLINE BulletVehicleTuning &get_tuning();

public:
  INLINE btRaycastVehicle *get_vehicle() const;

  void sync_b2p();

private:
  btRaycastVehicle *_vehicle;
  btVehicleRaycaster *_raycaster;

  BulletVehicleTuning _tuning;

  static btVector3 get_axis(int idx);

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BulletVehicle", 
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "bulletVehicle.I"

#endif // __BULLET_VEHICLE_H__
