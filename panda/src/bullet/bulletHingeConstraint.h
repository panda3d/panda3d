/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletHingeConstraint.h
 * @author enn0x
 * @date 2010-03-01
 */

#ifndef __BULLET_HINGE_CONSTRAINT_H__
#define __BULLET_HINGE_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletConstraint.h"

#include "luse.h"

class BulletRigidBodyNode;

/**
 * The hinge constraint lets two bodies rotate around a given axis while
 * adhering to specified limits.  It's motor can apply angular force to them.
 */
class EXPCL_PANDABULLET BulletHingeConstraint : public BulletConstraint {
PUBLISHED:
  explicit BulletHingeConstraint(const BulletRigidBodyNode *node_a,
                                 const LPoint3 &pivot_a,
                                 const LVector3 &axis_a,
                                 bool use_frame_a=false);
  explicit BulletHingeConstraint(const BulletRigidBodyNode *node_a,
                                 const BulletRigidBodyNode *node_b,
                                 const LPoint3 &pivot_a,
                                 const LPoint3 &pivot_b,
                                 const LVector3 &axis_a,
                                 const LVector3 &axis_b,
                                 bool use_frame_a=false);

  explicit BulletHingeConstraint(const BulletRigidBodyNode *node_a,
                                 const TransformState *ts_a,
                                 bool use_frame_a=false);
  explicit BulletHingeConstraint(const BulletRigidBodyNode *node_a,
                                 const BulletRigidBodyNode *node_b,
                                 const TransformState *ts_a,
                                 const TransformState *ts_b,
                                 bool use_frame_a=false);

  INLINE ~BulletHingeConstraint();

  PN_stdfloat get_hinge_angle();
  PN_stdfloat get_lower_limit() const;
  PN_stdfloat get_upper_limit() const;
  bool get_angular_only() const;

  void set_angular_only(bool value);
  void set_limit(PN_stdfloat low, PN_stdfloat high, PN_stdfloat softness=0.9f, PN_stdfloat bias=0.3f, PN_stdfloat relaxation=1.0f);
  void set_axis(const LVector3 &axis);

  void enable_angular_motor(bool enable, PN_stdfloat target_velocity, PN_stdfloat max_impulse);
  void enable_motor(bool enable);
  void set_max_motor_impulse(PN_stdfloat max_impulse);
  void set_motor_target(const LQuaternion &quat, PN_stdfloat dt);
  void set_motor_target(PN_stdfloat target_angle, PN_stdfloat dt);

  void set_frames(const TransformState *ts_a, const TransformState *ts_b);
  CPT(TransformState) get_frame_a() const;
  CPT(TransformState) get_frame_b() const;

  MAKE_PROPERTY(hinge_angle, get_hinge_angle);
  MAKE_PROPERTY(lower_limit, get_lower_limit);
  MAKE_PROPERTY(upper_limit, get_upper_limit);
  MAKE_PROPERTY(angular_only, get_angular_only, set_angular_only);
  MAKE_PROPERTY(frame_a, get_frame_a);
  MAKE_PROPERTY(frame_b, get_frame_b);

public:
  virtual btTypedConstraint *ptr() const;

private:
  btHingeConstraint *_constraint;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletConstraint::init_type();
    register_type(_type_handle, "BulletHingeConstraint",
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

#include "bulletHingeConstraint.I"

#endif // __BULLET_HINGE_CONSTRAINT_H__
