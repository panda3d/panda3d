/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletShape.h
 * @author enn0x
 * @date 2010-01-23
 */

#ifndef __BULLET_SHAPE_H__
#define __BULLET_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"

#include "typedWritableReferenceCount.h"
#include "boundingSphere.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletShape : public TypedWritableReferenceCount {
protected:
  INLINE BulletShape() {};

PUBLISHED:
  INLINE virtual ~BulletShape();

  bool is_polyhedral() const;
  bool is_convex() const;
  bool is_convex_2d() const;
  bool is_concave() const;
  bool is_infinite() const;
  bool is_non_moving() const;
  bool is_soft_body() const;

  void set_margin(PN_stdfloat margin);
  const char *get_name() const;

  PN_stdfloat get_margin() const;

  BoundingSphere get_shape_bounds() const;
  
  MAKE_PROPERTY(polyhedral, is_polyhedral);
  MAKE_PROPERTY(convex, is_convex);
  MAKE_PROPERTY(convex_2d, is_convex_2d);
  MAKE_PROPERTY(concave, is_concave);
  MAKE_PROPERTY(infinite, is_infinite);
  MAKE_PROPERTY(non_moving, is_non_moving);
  MAKE_PROPERTY(soft_body, is_soft_body);
  MAKE_PROPERTY(margin, get_margin, set_margin);
  MAKE_PROPERTY(name, get_name);
  MAKE_PROPERTY(shape_bounds, get_shape_bounds);

public:
  virtual btCollisionShape *ptr() const = 0;
  LVecBase3 get_local_scale() const;
  void set_local_scale(const LVecBase3 &scale);
  void do_set_local_scale(const LVecBase3 &scale);

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
