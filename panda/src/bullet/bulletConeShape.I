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

 */
INLINE BulletConeShape::
~BulletConeShape() {

  delete _shape;
}

/**

 */
INLINE BulletConeShape::
BulletConeShape(const BulletConeShape &copy) :
  _shape(copy._shape) {
}

/**

 */
INLINE void BulletConeShape::
operator = (const BulletConeShape &copy) {
  _shape = copy._shape;
}

/**

 */
INLINE PN_stdfloat BulletConeShape::
get_radius() const {

  return (PN_stdfloat)_shape->getRadius();
}

/**

 */
INLINE PN_stdfloat BulletConeShape::
get_height() const {

  return (PN_stdfloat)_shape->getHeight();
}
