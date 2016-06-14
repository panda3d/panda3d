/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletHeightfieldShape.h
 * @author enn0x
 * @date 2010-02-05
 */

#ifndef __BULLET_HEIGHTFIELD_SHAPE_H__
#define __BULLET_HEIGHTFIELD_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

#include "pnmImage.h"
#include "pfmFile.h"
#include "dynamicHeightfield.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletHeightfieldShape : public BulletShape, public DynamicHeightfield::Observer {

PUBLISHED:
  BulletHeightfieldShape(const PNMImage &image, PN_stdfloat max_height, BulletUpAxis up=Z_up);
  BulletHeightfieldShape(const PfmFile &pfm, PN_stdfloat max_height, bool STM=true, BulletUpAxis up=Z_up);
  INLINE BulletHeightfieldShape(const BulletHeightfieldShape &copy);
  INLINE void operator = (const BulletHeightfieldShape &copy);
  INLINE ~BulletHeightfieldShape();

  void set_use_diamond_subdivision(bool flag=true);

  INLINE int get_x_size() const;
  INLINE int get_y_size() const;

  INLINE void set_dynamic_heightfield(DynamicHeightfield* dynamic_hf);

public:
  virtual btCollisionShape *ptr() const;
  void on_change();

private:
  void update_region(const LVector4i &corners, const PfmFile &field);
  void sample_regular(const PfmFile &pfm);
  PT(DynamicHeightfield) _dynamic_hf;
  int _x_size, _y_size;
  PN_stdfloat _max_height;
  vector_float _data;
  btHeightfieldTerrainShape *_shape;

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
