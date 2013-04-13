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
#include "luse.h"

class BulletWorld;
class BulletRigidBodyNode;
class BulletWheel;

////////////////////////////////////////////////////////////////////
//       Class : BulletVehicleTuning
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletVehicleTuning {

PUBLISHED:
  INLINE void set_suspension_stiffness(PN_stdfloat value);
  INLINE void set_suspension_compression(PN_stdfloat value);
  INLINE void set_suspension_damping(PN_stdfloat value);
  INLINE void set_max_suspension_travel_cm(PN_stdfloat value);
  INLINE void set_friction_slip(PN_stdfloat value);
  INLINE void set_max_suspension_force(PN_stdfloat value);

  INLINE PN_stdfloat get_suspension_stiffness() const;
  INLINE PN_stdfloat get_suspension_compression() const;
  INLINE PN_stdfloat get_suspension_damping() const;
  INLINE PN_stdfloat get_max_suspension_travel_cm() const;
  INLINE PN_stdfloat get_friction_slip() const;
  INLINE PN_stdfloat get_max_suspension_force() const;

private:
  btRaycastVehicle::btVehicleTuning _;

  friend class BulletVehicle;
};

////////////////////////////////////////////////////////////////////
//       Class : BulletVehicle
// Description : Simulates a raycast vehicle which casts a ray per
//               wheel at the ground as a cheap replacement for
//               complex suspension simulation. The suspension can
//               be tuned in various ways. It is possible to add a
//               (probably) arbitrary number of wheels.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletVehicle : public TypedReferenceCount {

PUBLISHED:
  BulletVehicle(BulletWorld *world, BulletRigidBodyNode *chassis);
  INLINE ~BulletVehicle();

  void set_coordinate_system(BulletUpAxis up);
  void set_steering_value(PN_stdfloat steering, int idx);
  void set_brake(PN_stdfloat brake, int idx);
  void set_pitch_control(PN_stdfloat pitch);

  BulletRigidBodyNode *get_chassis();
  PN_stdfloat get_current_speed_km_hour() const;
  PN_stdfloat get_steering_value(int idx) const;
  LVector3 get_forward_vector() const;

  void reset_suspension();
  void apply_engine_force(PN_stdfloat force, int idx);

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
