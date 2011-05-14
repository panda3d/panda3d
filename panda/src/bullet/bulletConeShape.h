// Filename: bulletConeShape.h
// Created by:  enn0x (24Jan10)
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

#ifndef __BULLET_CONE_SHAPE_H__
#define __BULLET_CONE_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletConeShape
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletConeShape : public BulletShape {

PUBLISHED:
  BulletConeShape(float radius, float height, BulletUpAxis up=Z_up);
  INLINE ~BulletConeShape();

  INLINE float get_radius() const;
  INLINE float get_height() const;

public:
  virtual btCollisionShape *ptr() const;

private:
  btConeShape *_shape;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletConeShape", 
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

#include "bulletConeShape.I"

#endif // __BULLET_CONE_SHAPE_H__
