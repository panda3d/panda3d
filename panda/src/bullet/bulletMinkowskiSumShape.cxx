/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletMinkowskiSumShape.cxx
 * @author enn0x
 * @date 2013-08-15
 */

#include "bulletMinkowskiSumShape.h"

TypeHandle BulletMinkowskiSumShape::_type_handle;

/**
 *
 */
BulletMinkowskiSumShape::
BulletMinkowskiSumShape(const BulletShape *shape_a, const BulletShape *shape_b) {

  nassertv(shape_a->is_convex());
  nassertv(shape_b->is_convex());

  const btConvexShape *ptr_a = (const btConvexShape *)shape_a->ptr();
  const btConvexShape *ptr_b = (const btConvexShape *)shape_b->ptr();

  _shape = new btMinkowskiSumShape(ptr_a, ptr_b);
  _shape->setUserPointer(this);

  _shape_a = shape_a;
  _shape_b = shape_b;
}

/**
 *
 */
btCollisionShape *BulletMinkowskiSumShape::
ptr() const {

  return _shape;
}
