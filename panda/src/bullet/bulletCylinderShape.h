// Filename: bulletCylinderShape.h
// Created by:  enn0x (17Feb10)
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

#ifndef __BULLET_CYLINDER_SHAPE_H__
#define __BULLET_CYLINDER_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletCylinderShape
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletCylinderShape : public BulletShape {

PUBLISHED:
  BulletCylinderShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up=Z_up);
  BulletCylinderShape(const LVector3 &half_extents, BulletUpAxis up=Z_up);
  INLINE BulletCylinderShape(const BulletCylinderShape &copy);
  INLINE void operator = (const BulletCylinderShape &copy);
  INLINE ~BulletCylinderShape();

  INLINE PN_stdfloat get_radius() const;
  INLINE LVecBase3 get_half_extents_without_margin() const;
  INLINE LVecBase3 get_half_extents_with_margin() const;

public:
  virtual btCollisionShape *ptr() const;

private:
  btCylinderShape *_shape;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletCylinderShape", 
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

#include "bulletCylinderShape.I"

#endif // __BULLET_CYLINDER_SHAPE_H__
