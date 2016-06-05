/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletHeightfieldShape.I
 * @author enn0x
 * @date 2010-02-05
 */

/**
 *
 */
INLINE BulletHeightfieldShape::
~BulletHeightfieldShape() {

  delete _shape;
}

/**
 *
 */
INLINE BulletHeightfieldShape::
BulletHeightfieldShape(const BulletHeightfieldShape &copy) :

  _shape(copy._shape),
  _y_size(copy._y_size),
  _x_size(copy._x_size),
  _max_height(copy._max_height),
  _data(copy._data) {}

/**
 *
 */
INLINE void BulletHeightfieldShape::
operator = (const BulletHeightfieldShape &copy) {

  _shape = copy._shape;
  _y_size = copy._y_size;
  _x_size = copy._x_size;
  _max_height = copy._max_height;
  _data = copy._data;
}
