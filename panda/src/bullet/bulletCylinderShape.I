/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCylinderShape.I
 * @author enn0x
 * @date 2010-02-17
 */

/**
 *
 */
INLINE BulletCylinderShape::
~BulletCylinderShape() {

  delete _shape;
}

/**
 *
 */
INLINE BulletCylinderShape::
BulletCylinderShape(const BulletCylinderShape &copy) :
  _shape(copy._shape), _half_extents(copy._half_extents) {
}

/**
 *
 */
INLINE void BulletCylinderShape::
operator = (const BulletCylinderShape &copy) {
  _shape = copy._shape;
  _half_extents = copy._half_extents;
}

/**
 *
 */
INLINE PN_stdfloat BulletCylinderShape::
get_radius() const {

  return (PN_stdfloat)_shape->getRadius();
}

/**
 *
 */
INLINE LVecBase3 BulletCylinderShape::
get_half_extents_without_margin() const {

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithoutMargin());
}

/**
 *
 */
INLINE LVecBase3 BulletCylinderShape::
get_half_extents_with_margin() const {

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithMargin());
}
