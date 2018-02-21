/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSliderConstraint.h
 * @author enn0x
 * @date 2010-03-01
 */

#ifndef __BULLET_SLIDER_CONSTRAINT_H__
#define __BULLET_SLIDER_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletConstraint.h"

#include "transformState.h"

class BulletRigidBodyNode;

/**
 *
 */
class EXPCL_PANDABULLET BulletSliderConstraint : public BulletConstraint {
PUBLISHED:
  explicit BulletSliderConstraint(const BulletRigidBodyNode *node_a,
                                  const TransformState *frame_a,
                                  bool useFrame_a);
  explicit BulletSliderConstraint(const BulletRigidBodyNode *node_a,
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
  CPT(TransformState) get_frame_a() const;
  CPT(TransformState) get_frame_b() const;

  MAKE_PROPERTY(linear_pos, get_linear_pos);
  MAKE_PROPERTY(angular_pos, get_angular_pos);
  MAKE_PROPERTY(lower_linear_limit, get_lower_linear_limit, set_lower_linear_limit);
  MAKE_PROPERTY(upper_linear_limit, get_upper_linear_limit, set_upper_linear_limit);
  MAKE_PROPERTY(lower_angular_limit, get_lower_angular_limit, set_lower_angular_limit);
  MAKE_PROPERTY(upper_angular_limit, get_upper_angular_limit, set_upper_angular_limit);
  MAKE_PROPERTY(powered_linear_motor, get_powered_linear_motor, set_powered_linear_motor);
  MAKE_PROPERTY(target_linear_motor_velocity, get_target_linear_motor_velocity, set_target_linear_motor_velocity);
  MAKE_PROPERTY(max_linear_motor_force, get_max_linear_motor_force, set_max_linear_motor_force);
  MAKE_PROPERTY(powered_angular_motor, get_powered_angular_motor, set_powered_angular_motor);
  MAKE_PROPERTY(target_angular_motor_velocity, get_target_angular_motor_velocity, set_target_angular_motor_velocity);
  MAKE_PROPERTY(max_angular_motor_force, get_max_angular_motor_force, set_max_angular_motor_force);
  MAKE_PROPERTY(frame_a, get_frame_a);
  MAKE_PROPERTY(frame_b, get_frame_b);

public:
  virtual btTypedConstraint *ptr() const;

private:
  btSliderConstraint *_constraint;

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
