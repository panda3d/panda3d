/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletMultiSphereShape.I
 * @author enn0x
 * @date 2012-01-04
 */

/**
 *
 */
INLINE BulletMultiSphereShape::
~BulletMultiSphereShape() {

  delete _shape;
}

/**
 *
 */
INLINE BulletMultiSphereShape::
BulletMultiSphereShape(const BulletMultiSphereShape &copy) :
  _shape(copy._shape) {
}

/**
 *
 */
INLINE void BulletMultiSphereShape::
operator = (const BulletMultiSphereShape &copy) {
  _shape = copy._shape;
}

/**
 *
 */
INLINE int BulletMultiSphereShape::
get_sphere_count() const {

  return _shape->getSphereCount();
}

/**
 *
 */
INLINE LPoint3 BulletMultiSphereShape::
get_sphere_pos(int index) const {

  nassertr(index >=0 && index <_shape->getSphereCount(), LPoint3::zero());
  return btVector3_to_LPoint3(_shape->getSpherePosition(index));
}

/**
 *
 */
INLINE PN_stdfloat BulletMultiSphereShape::
get_sphere_radius(int index) const {

  nassertr(index >=0 && index <_shape->getSphereCount(), 0.0);
  return (PN_stdfloat)_shape->getSphereRadius(index);
}
