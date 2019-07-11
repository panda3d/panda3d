/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletVehicle.h
 * @author enn0x
 * @date 2010-02-16
 */

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

/**
 *
 */
class EXPCL_PANDABULLET BulletVehicleTuning {

PUBLISHED:
  void set_suspension_stiffness(PN_stdfloat value);
  void set_suspension_compression(PN_stdfloat value);
  void set_suspension_damping(PN_stdfloat value);
  void set_max_suspension_travel_cm(PN_stdfloat value);
  void set_friction_slip(PN_stdfloat value);
  void set_max_suspension_force(PN_stdfloat value);

  PN_stdfloat get_suspension_stiffness() const;
  PN_stdfloat get_suspension_compression() const;
  PN_stdfloat get_suspension_damping() const;
  PN_stdfloat get_max_suspension_travel_cm() const;
  PN_stdfloat get_friction_slip() const;
  PN_stdfloat get_max_suspension_force() const;

  MAKE_PROPERTY(suspension_stiffness, get_suspension_stiffness, set_suspension_stiffness);
  MAKE_PROPERTY(suspension_compression, get_suspension_compression, set_suspension_compression);
  MAKE_PROPERTY(suspension_damping, get_suspension_damping, set_suspension_damping);
  MAKE_PROPERTY(max_suspension_travel_cm, get_max_suspension_travel_cm, set_max_suspension_travel_cm);
  MAKE_PROPERTY(friction_slip, get_friction_slip, set_friction_slip);
  MAKE_PROPERTY(max_suspension_force, get_max_suspension_force, set_max_suspension_force);

private:
  btRaycastVehicle::btVehicleTuning _;

  friend class BulletVehicle;
};

/**
 * Simulates a raycast vehicle which casts a ray per wheel at the ground as a
 * cheap replacement for complex suspension simulation.  The suspension can be
 * tuned in various ways.  It is possible to add a (probably) arbitrary number
 * of wheels.
 */
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
  BulletWheel create_wheel(PN_stdfloat suspension_rest_length=0.4);

  int get_num_wheels() const;
  BulletWheel get_wheel(int idx) const;
  MAKE_SEQ(get_wheels, get_num_wheels, get_wheel);

  // Tuning
  INLINE BulletVehicleTuning &get_tuning();

  MAKE_PROPERTY(chassis, get_chassis);
  MAKE_PROPERTY(current_speed_km_hour, get_current_speed_km_hour);
  MAKE_PROPERTY(forward_vector, get_forward_vector);
  MAKE_SEQ_PROPERTY(wheels, get_num_wheels, get_wheel);
  MAKE_PROPERTY(tuning, get_tuning);

public:
  INLINE btRaycastVehicle *get_vehicle() const;
  BulletRigidBodyNode *do_get_chassis();

  void do_sync_b2p();

private:
  btRaycastVehicle *_vehicle;
  btVehicleRaycaster *_raycaster;

  BulletVehicleTuning _tuning;

  static btVector3 get_axis(int idx);

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
