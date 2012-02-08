// Filename: bulletHeightfieldShape.h
// Created by:  enn0x (05Feb10)
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

#ifndef __BULLET_HEIGHTFIELD_SHAPE_H__
#define __BULLET_HEIGHTFIELD_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

#include "pnmImage.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletHeightfieldShape
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletHeightfieldShape : public BulletShape {

PUBLISHED:
  BulletHeightfieldShape(const PNMImage &image, PN_stdfloat max_height, BulletUpAxis up=Z_up);
  INLINE BulletHeightfieldShape(const BulletHeightfieldShape &copy);
  INLINE void operator = (const BulletHeightfieldShape &copy);
  INLINE ~BulletHeightfieldShape();

  void set_use_diamond_subdivision(bool flag=true);

public:
  virtual btCollisionShape *ptr() const;

private:
  int _num_rows;
  int _num_cols;
  float *_data;
  btHeightfieldTerrainShape *_shape;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletHeightfieldShape", 
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

#include "bulletHeightfieldShape.I"

#endif // __BULLET_HEIGHTFIELD_SHAPE_H__
