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
  _x_size(copy._x_size),
  _y_size(copy._y_size),
  _max_height(copy._max_height),
  _data(copy._data) {}

/**
 *
 */
INLINE void BulletHeightfieldShape::
operator = (const BulletHeightfieldShape &copy) {

  _shape = copy._shape;
  _x_size = copy._x_size;
  _y_size = copy._y_size;
  _max_height = copy._max_height;
  _data = copy._data;
}

/**
 *
 */
INLINE int BulletHeightfieldShape::
get_x_size() const {

  return _x_size;
}

/**
 *
 */
INLINE int BulletHeightfieldShape::
get_y_size() const {

  return _y_size;
}

/**
 * @brief Sets a DynamicHeightfield which will propagate changes made to it.
 * @details It's recommended to construct BulletHeightfieldShape with a 
 *   heightfield and STM sampling. Then, construct DynamicHeightfield, 
 *   call read() using the same heightfield and use this method to hook them up.
 */

INLINE void BulletHeightfieldShape::
set_dynamic_heightfield(DynamicHeightfield* dynamic_hf) {
  if (_x_size != dynamic_hf->get_x_size()+1 || _y_size != dynamic_hf->get_y_size()+1) {
    bullet_cat.error() << "Size mismatch between BulletHeightfieldShape and DynamicHeightfield! DynamicHeightfield was not set." << endl;
    return;
  }
  _dynamic_hf = dynamic_hf;
  _dynamic_hf->add_observer(this);
}