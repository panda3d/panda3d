// Filename: bulletSliderConstraint.h
// Created by:  enn0x (01Mar10)
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

#ifndef __BULLET_SLIDER_CONSTRAINT_H__
#define __BULLET_SLIDER_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletConstraint.h"

#include "transformState.h"

class BulletRigidBodyNode;

////////////////////////////////////////////////////////////////////
//       Class : BulletSliderConstraint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletSliderConstraint : public BulletConstraint {

PUBLISHED:
  BulletSliderConstraint(const BulletRigidBodyNode *node_a, 
                         const TransformState *frame_a,
                         bool useFrame_a);
  BulletSliderConstraint(const BulletRigidBodyNode *node_a,
                         const BulletRigidBodyNode *node_b,
                         const TransformState *frame_a,
                         const TransformState *frame_b,
                         bool use_frame_a);
  INLINE ~BulletSliderConstraint();

  PN_stdfloat get_linear_pos() const;
  PN_stdfloat get_angular_pos() const;

  // Limits
  PN_stdfloat get_lower_linear_limit() const;
  PN_stdfloat get_upper_linear_limit() const;
  PN_stdfloat get_lower_angular_limit() const;
  PN_stdfloat get_upper_angular_limit() const;
  void set_lower_linear_limit(PN_stdfloat value);
  void set_upper_linear_limit(PN_stdfloat value);
  void set_lower_angular_limit(PN_stdfloat value);
  void set_upper_angular_limit(PN_stdfloat value);

  // Linear motor
  void set_powered_linear_motor(bool on);
  void set_target_linear_motor_velocity (PN_stdfloat target_velocity);
  void set_max_linear_motor_force(PN_stdfloat max_force);
  bool get_powered_linear_motor() const;
  PN_stdfloat get_target_linear_motor_velocity() const;
  PN_stdfloat get_max_linear_motor_force() const;

  // Angular motor
  void set_powered_angular_motor(bool on);
  void set_target_angular_motor_velocity (PN_stdfloat target_velocity);
  void set_max_angular_motor_force(PN_stdfloat max_force);
  bool get_powered_angular_motor() const;
  PN_stdfloat get_target_angular_motor_velocity() const;
  PN_stdfloat get_max_angular_motor_force() const;

  // Frames
  void set_frames(const TransformState *ts_a, const TransformState *ts_b);
  INLINE CPT(TransformState) get_frame_a() const;
  INLINE CPT(TransformState) get_frame_b() const;

public:
  virtual btTypedConstraint *ptr() const;

private:
  btSliderConstraint *_constraint;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletConstraint::init_type();
    register_type(_type_handle, "BulletSliderConstraint", 
                  BulletConstraint::get_class_type());
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

#include "bulletSliderConstraint.I"

#endif // __BULLET_SLIDER_CONSTRAINT_H__
