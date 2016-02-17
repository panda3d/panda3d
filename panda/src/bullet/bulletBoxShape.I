/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletBoxShape.I
 * @author enn0x
 * @date 2010-01-24
 */

/**
 *
 */
INLINE BulletBoxShape::
~BulletBoxShape() {

  delete _shape;
}

/**
 *
 */
INLINE BulletBoxShape::
BulletBoxShape(const BulletBoxShape &copy) :
  _shape(copy._shape) {
}

/**
 *
 */
INLINE void BulletBoxShape::
operator = (const BulletBoxShape &copy) {
  _shape = copy._shape;
}
