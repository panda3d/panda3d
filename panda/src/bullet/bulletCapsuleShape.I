/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCapsuleShape.I
 * @author enn0x
 * @date 2010-01-27
 */

/**
 *
 */
INLINE BulletCapsuleShape::
~BulletCapsuleShape() {

  delete _shape;
}

/**
 *
 */
INLINE BulletCapsuleShape::
BulletCapsuleShape(const BulletCapsuleShape &copy) :
  _shape(copy._shape), _radius(copy._radius), _height(copy._height) {
}

/**
 *
 */
INLINE void BulletCapsuleShape::
operator = (const BulletCapsuleShape &copy) {
  _shape = copy._shape;
  _radius = copy._radius;
  _height = copy._height;
}

/**
 *
 */
INLINE PN_stdfloat BulletCapsuleShape::
get_radius() const {

  return (PN_stdfloat)_shape->getRadius();
}

/**
 *
 */
INLINE PN_stdfloat BulletCapsuleShape::
get_half_height() const {

  return (PN_stdfloat)_shape->getHalfHeight();
}
