// Filename: bulletHingeConstraint.h
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

#ifndef __BULLET_HINGE_CONSTRAINT_H__
#define __BULLET_HINGE_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletConstraint.h"

#include "lpoint3.h"
#include "lvector3.h"

class BulletRigidBodyNode;

////////////////////////////////////////////////////////////////////
//       Class : BulletHingeConstraint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletHingeConstraint : public BulletConstraint {

PUBLISHED:
  BulletHingeConstraint(const BulletRigidBodyNode *node_a, 
                        const LPoint3f &pivot_a,
                        const LVector3f &axis_a,
                        bool use_frame_a=false);
  BulletHingeConstraint(const BulletRigidBodyNode *node_a,
                        const BulletRigidBodyNode *node_b,
                        const LPoint3f &pivot_a,
                        const LPoint3f &pivot_b,
                        const LVector3f &axis_a,
                        const LVector3f &axis_b,
                        bool use_frame_a=false);
  INLINE ~BulletHingeConstraint();

  float get_hinge_angle();
  float get_lower_limit() const;
  float get_upper_limit() const;
  bool get_angular_only() const;

  void set_angular_only(bool value);
  void set_limit(float low, float high, float softness=0.9f, float bias=0.3f, float relaxation=1.0f);
  void set_axis(const LVector3f &axis);

public:
  virtual btTypedConstraint *ptr() const;

private:
  btHingeConstraint *_constraint;

////////////////////////////////////////////////////////////////////
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
