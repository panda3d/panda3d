// Filename: bulletCapsuleShape.h
// Created by:  enn0x (27Jan10)
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

#ifndef __BULLET_CAPSULE_SHAPE_H__
#define __BULLET_CAPSULE_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletCapsuleShape
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletCapsuleShape : public BulletShape {

PUBLISHED:
  BulletCapsuleShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up=Z_up);
  INLINE BulletCapsuleShape(const BulletCapsuleShape &copy);
  INLINE void operator = (const BulletCapsuleShape &copy);
  INLINE ~BulletCapsuleShape();

  INLINE PN_stdfloat get_radius() const;
  INLINE PN_stdfloat get_half_height() const;

public:
  virtual btCollisionShape *ptr() const;

private:
  btCapsuleShape *_shape;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletCapsuleShape", 
                  BulletShape::get_class_type());
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

#include "bulletCapsuleShape.I"

#endif // __BULLET_CAPSULE_SHAPE_H__
