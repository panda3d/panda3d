/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletGenericConstraint.h
 * @author enn0x
 * @date 2010-03-02
 */

#ifndef __BULLET_GENERIC_CONSTRAINT_H__
#define __BULLET_GENERIC_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletConstraint.h"
#include "bulletRotationalLimitMotor.h"
#include "bulletTranslationalLimitMotor.h"

#include "transformState.h"
#include "luse.h"

class BulletRigidBodyNode;

/**
 *
 */
class EXPCL_PANDABULLET BulletGenericConstraint : public BulletConstraint {
PUBLISHED:
  explicit BulletGenericConstraint(const BulletRigidBodyNode *node_a,
                                   const TransformState *frame_a,
                                   bool use_frame_a);
  explicit BulletGenericConstraint(const BulletRigidBodyNode *node_a,
                                   const BulletRigidBodyNode *node_b,
                                   const TransformState *frame_a,
                                   const TransformState *frame_b,
                                   bool use_frame_a);
  INLINE ~BulletGenericConstraint();

  // Geometry
  LVector3 get_axis(int axis) const;
  PN_stdfloat get_pivot(int axis) const;
  PN_stdfloat get_angle(int axis) const;

  // Limit
  void set_linear_limit(int axis, PN_stdfloat low, PN_stdfloat high);
  void set_angular_limit(int axis, PN_stdfloat low, PN_stdfloat high);

  // Motors
  BulletRotationalLimitMotor get_rotational_limit_motor(int axis);
  BulletTranslationalLimitMotor get_translational_limit_motor();

  // Frames
  void set_frames(const TransformState *ts_a, const TransformState *ts_b);
  CPT(TransformState) get_frame_a() const;
  CPT(TransformState) get_frame_b() const;

  MAKE_PROPERTY(translational_limit_motor, get_translational_limit_motor);
  MAKE_PROPERTY(frame_a, get_frame_a);
  MAKE_PROPERTY(frame_b, get_frame_b);

public:
  virtual btTypedConstraint *ptr() const;

private:
  btGeneric6DofConstraint *_constraint;

// TODO btRotationalLimitMotor *  getRotationalLimitMotor (int index)

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletConstraint::init_type();
    register_type(_type_handle, "BulletGenericConstraint",
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

#include "bulletGenericConstraint.I"

#endif // __BULLET_GENERIC_CONSTRAINT_H__
