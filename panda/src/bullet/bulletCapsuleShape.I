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
 * Only used by make_from_bam.
 */
INLINE BulletCapsuleShape::
BulletCapsuleShape() :
  _shape(nullptr),
  _radius(0),
  _height(0),
  _up(X_up) {
}

/**
 *
 */
INLINE BulletCapsuleShape::
~BulletCapsuleShape() {

  delete _shape;
}

/**
 * Returns the radius that was used to construct this capsule.
 */
INLINE PN_stdfloat BulletCapsuleShape::
get_radius() const {
  return _radius;
}

/**
 * Returns half of get_height().
 * @deprecated see get_height() instead.
 */
INLINE PN_stdfloat BulletCapsuleShape::
get_half_height() const {
  return _height * 0.5;
}

/**
 * Returns the height that was used to construct this capsule.
 */
INLINE PN_stdfloat BulletCapsuleShape::
get_height() const {
  return _height;
}
