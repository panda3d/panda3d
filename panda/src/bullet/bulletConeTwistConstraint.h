/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConeTwistConstraint.h
 * @author enn0x
 * @date 2010-03-01
 */

#ifndef __BULLET_CONE_TWIST_CONSTRAINT_H__
#define __BULLET_CONE_TWIST_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletConstraint.h"

#include "transformState.h"

class BulletRigidBodyNode;

/**
 *
 */
class EXPCL_PANDABULLET BulletConeTwistConstraint : public BulletConstraint {

PUBLISHED:
  explicit BulletConeTwistConstraint(const BulletRigidBodyNode *node_a,
                                     const TransformState *frame_a);
  explicit BulletConeTwistConstraint(const BulletRigidBodyNode *node_a,
                                     const BulletRigidBodyNode *node_b,
                                     const TransformState *frame_a,
                                     const TransformState *frame_b);
  INLINE ~BulletConeTwistConstraint();

  void set_limit(int index, PN_stdfloat value);
  void set_limit(PN_stdfloat swing1, PN_stdfloat swing2, PN_stdfloat twist, PN_stdfloat softness=1.0f, PN_stdfloat bias=0.3f, PN_stdfloat relaxation=1.0f);

  void set_damping(PN_stdfloat damping);

  PN_stdfloat get_fix_threshold() const;
  void set_fix_threshold(PN_stdfloat threshold);

  void enable_motor(bool enable);
  void set_max_motor_impulse(PN_stdfloat max_impulse);
  void set_max_motor_impulse_normalized(PN_stdfloat max_impulse);
  void set_motor_target(const LQuaternion &quat);
  void set_motor_target_in_constraint_space(const LQuaternion &quat);

  void set_frames(const TransformState *ts_a, const TransformState *ts_b);
  CPT(TransformState) get_frame_a() const;
  CPT(TransformState) get_frame_b() const;

  MAKE_PROPERTY(fix_threshold, get_fix_threshold, set_fix_threshold);
  MAKE_PROPERTY(frame_a, get_frame_a);
  MAKE_PROPERTY(frame_b, get_frame_b);

public:
  virtual btTypedConstraint *ptr() const;

private:
  btConeTwistConstraint *_constraint;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletConstraint::init_type();
    register_type(_type_handle, "BulletConeTwistConstraint",
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

#include "bulletConeTwistConstraint.I"

#endif // __BULLET_CONE_TWIST_CONSTRAINT_H__
