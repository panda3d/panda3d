/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyShape.h
 * @author enn0x
 * @date 2010-05-06
 */

#ifndef __BULLET_SOFT_BODY_SHAPE_H__
#define __BULLET_SOFT_BODY_SHAPE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bulletShape.h"

class BulletSoftBodyNode;

/**
 *
 */
class EXPCL_PANDABULLET BulletSoftBodyShape : public BulletShape {

PUBLISHED:
  INLINE ~BulletSoftBodyShape();

  BulletSoftBodyNode *get_body() const;

  MAKE_PROPERTY(body, get_body);

public:
  BulletSoftBodyShape(btSoftBodyCollisionShape *shapePtr);

  virtual btCollisionShape *ptr() const;

private:
  btSoftBodyCollisionShape *_shape;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletShape::init_type();
    register_type(_type_handle, "BulletSoftBodyShape",
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

#include "bulletSoftBodyShape.I"

#endif // __BULLET_SOFT_BODY_SHAPE_H__
