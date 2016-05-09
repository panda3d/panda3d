/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletMinkowskiSumShape.h
 * @author enn0x
 * @date 2013-08-15
 */

#ifndef __BULLET_MINKOWSKI_SUM_SHAPE_H__
#define __BULLET_MINKOWSKI_SUM_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

#include "transformState.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletMinkowskiSumShape : public BulletShape {

PUBLISHED:
  BulletMinkowskiSumShape(const BulletShape *shape_a, const BulletShape *shape_b);
  INLINE BulletMinkowskiSumShape(const BulletMinkowskiSumShape &copy);
  INLINE void operator = (const BulletMinkowskiSumShape &copy);
  INLINE ~BulletMinkowskiSumShape();

  INLINE void set_transform_a(const TransformState *ts);
  INLINE void set_transform_b(const TransformState *ts);
  INLINE CPT(TransformState) get_transform_a() const;
  INLINE CPT(TransformState) get_transform_b() const;

  INLINE const BulletShape *get_shape_a() const;
  INLINE const BulletShape *get_shape_b() const;

  INLINE PN_stdfloat get_margin() const;

  MAKE_PROPERTY(transform_a, get_transform_a, set_transform_a);
  MAKE_PROPERTY(transform_b, get_transform_b, set_transform_b);
  MAKE_PROPERTY(shape_a, get_shape_a);
  MAKE_PROPERTY(shape_b, get_shape_b);
  MAKE_PROPERTY(margin, get_margin);

public:
  virtual btCollisionShape *ptr() const;

private:
  btMinkowskiSumShape *_shape;

  CPT(BulletShape) _shape_a;
  CPT(BulletShape) _shape_b;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletMinkowskiSumShape",
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

#include "bulletMinkowskiSumShape.I"

#endif // __BULLET_MINKOWSKI_SUM_SHAPE_H__
