// Filename: bulletDistanceConstraint.h
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

#ifndef __BULLET_DISTANCE_CONSTRAINT_H__
#define __BULLET_DISTANCE_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletConstraint.h"

#include "lpoint3.h"

class BulletRigidBodyNode;

////////////////////////////////////////////////////////////////////
//       Class : BulletDistanceConstraint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletDistanceConstraint : public BulletConstraint {

PUBLISHED:
  BulletDistanceConstraint(const BulletRigidBodyNode *node_a, 
                           const LPoint3f &pivot_a);
  BulletDistanceConstraint(const BulletRigidBodyNode *node_a,
                           const BulletRigidBodyNode *node_b,
                           const LPoint3f &pivot_a,
                           const LPoint3f &pivot_b);
  INLINE ~BulletDistanceConstraint();

  void set_pivot_a(const LPoint3f &pivot_a);
  void set_pivot_b(const LPoint3f &pivot_b);

  LPoint3f get_pivot_in_a() const;
  LPoint3f get_pivot_in_b() const;

public:
  virtual btTypedConstraint *ptr() const;

private:
  btPoint2PointConstraint *_constraint;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletConstraint::init_type();
    register_type(_type_handle, "BulletDistanceConstraint", 
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

#include "bulletDistanceConstraint.I"

#endif // __BULLET_DISTANCE_CONSTRAINT_H__
