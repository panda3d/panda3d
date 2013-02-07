// Filename: bulletConstraint.h
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

#ifndef __BULLET_CONSTRAINT_H__
#define __BULLET_CONSTRAINT_H__

#include "pandabase.h"

#include "bullet_includes.h"

#include "typedReferenceCount.h"

class BulletRigidBodyNode;

////////////////////////////////////////////////////////////////////
//       Class : BulletConstraint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletConstraint : public TypedReferenceCount {

PUBLISHED:
  INLINE virtual ~BulletConstraint();

  BulletRigidBodyNode *get_rigid_body_a();
  BulletRigidBodyNode *get_rigid_body_b();

  void enable_feedback(bool value);
  void set_debug_draw_size(PN_stdfloat size);

  PN_stdfloat get_applied_impulse() const;
  PN_stdfloat get_debug_draw_size();

  INLINE void set_breaking_threshold(PN_stdfloat threshold);
  INLINE PN_stdfloat set_breaking_threshold() const;
  INLINE void set_enabled(bool enabled);
  INLINE bool is_enabled() const;

  enum ConstraintParam {
    CP_erp = 1,
    CP_stop_erp,
    CP_cfm,
    CP_stop_cfm
  };

  void set_param(ConstraintParam num, PN_stdfloat value, int axis=-1);
  PN_stdfloat get_param(ConstraintParam num, int axis=-1);

public:
  virtual btTypedConstraint *ptr() const = 0;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BulletConstraint", 
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

#include "bulletConstraint.I"

#endif // __BULLET_CONSTRAINT_H__
