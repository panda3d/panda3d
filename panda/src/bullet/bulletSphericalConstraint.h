// Filename: bulletSphericalConstraint.h
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

#ifndef __BULLET_SPHERICAL_CONSTRAINT_H__
#define __BULLET_SPHERICAL_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletConstraint.h"

#include "luse.h"

class BulletRigidBodyNode;

////////////////////////////////////////////////////////////////////
//       Class : BulletSphericalConstraint
// Description : A constraint between two rigid bodies, each with a
//               pivot point. The pivot points are described in the
//               body's local space. The constraint limits movement
//               of the two rigid bodies in such a way that the 
//               pivot points match in global space. The spherical
//               constraint can be seen as a "ball and socket"
//               joint.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletSphericalConstraint : public BulletConstraint {

PUBLISHED:
  BulletSphericalConstraint(const BulletRigidBodyNode *node_a, 
                            const LPoint3 &pivot_a);
  BulletSphericalConstraint(const BulletRigidBodyNode *node_a,
                            const BulletRigidBodyNode *node_b,
                            const LPoint3 &pivot_a,
                            const LPoint3 &pivot_b);
  INLINE ~BulletSphericalConstraint();

  // Pivots
  void set_pivot_a(const LPoint3 &pivot_a);
  void set_pivot_b(const LPoint3 &pivot_b);
  LPoint3 get_pivot_in_a() const;
  LPoint3 get_pivot_in_b() const;

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
    register_type(_type_handle, "BulletSphericalConstraint", 
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

#include "bulletSphericalConstraint.I"

#endif // __BULLET_SPHERICAL_CONSTRAINT_H__
