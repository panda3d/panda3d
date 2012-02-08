// Filename: bulletMultiSphereShape.h
// Created by:  enn0x (04Jan12)
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

#ifndef __BULLET_MULTI_SPHERE_SHAPE_H__
#define __BULLET_MULTI_SPHERE_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bulletShape.h"

#include "pta_LVecBase3.h"
#include "pta_stdfloat.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletMultiSphereShape
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletMultiSphereShape : public BulletShape {

PUBLISHED:
  BulletMultiSphereShape(const PTA_LVecBase3 &points, const PTA_stdfloat &radii);
  INLINE BulletMultiSphereShape(const BulletMultiSphereShape &copy);
  INLINE void operator = (const BulletMultiSphereShape &copy);
  INLINE ~BulletMultiSphereShape();

  INLINE int get_sphere_count() const;
  INLINE LPoint3 get_sphere_pos(int index) const;
  INLINE PN_stdfloat get_sphere_radius(int index) const;

public:
  virtual btCollisionShape *ptr() const;

private:
  btMultiSphereShape *_shape;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletMultiSphereShape", 
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

#include "bulletMultiSphereShape.I"

#endif // __BULLET_MULTI_SPHERE_SHAPE_H__
