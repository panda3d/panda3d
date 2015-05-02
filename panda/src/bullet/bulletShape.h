// Filename: bulletShape.h
// Created by:  enn0x (23Jan10)
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

#ifndef __BULLET_SHAPE_H__
#define __BULLET_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"

#include "typedReferenceCount.h"
#include "boundingSphere.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletShape
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletShape : public TypedWritableReferenceCount {
protected:
  INLINE BulletShape() {};

PUBLISHED:
  INLINE virtual ~BulletShape();

  INLINE bool is_polyhedral() const;
  INLINE bool is_convex() const;
  INLINE bool is_convex_2d() const;
  INLINE bool is_concave() const;
  INLINE bool is_infinite() const;
  INLINE bool is_non_moving() const;
  INLINE bool is_soft_body() const;

  void set_margin(PN_stdfloat margin);
  const char *get_name() const;

  PN_stdfloat get_margin() const;

  BoundingSphere get_shape_bounds() const;

public:
  virtual btCollisionShape *ptr() const = 0;

  LVecBase3 get_local_scale() const;
  void set_local_scale(const LVecBase3 &scale);

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "BulletShape", 
                  TypedWritableReferenceCount::get_class_type());
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

#include "bulletShape.I"

#endif // __BULLET_SHAPE_H__
