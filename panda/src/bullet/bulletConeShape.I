/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConeShape.I
 * @author enn0x
 * @date 2010-01-24
 */

/**
 * Only used by make_from_bam.
 */
INLINE BulletConeShape::
BulletConeShape() :
  _shape(nullptr),
  _radius(0),
  _height(0),
  _up(X_up) {
}

/**
 *
 */
INLINE BulletConeShape::
~BulletConeShape() {

  delete _shape;
}

/**
 * Returns the radius that was passed into the constructor.
 */
INLINE PN_stdfloat BulletConeShape::
get_radius() const {
  return _radius;
}

/**
 * Returns the height that was passed into the constructor.
 */
INLINE PN_stdfloat BulletConeShape::
get_height() const {
  return _height;
}
