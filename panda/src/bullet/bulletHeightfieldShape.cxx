/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletHeightfieldShape.cxx
 * @author enn0x
 * @date 2010-02-05
 */

#include "bulletHeightfieldShape.h"

TypeHandle BulletHeightfieldShape::_type_handle;

/**

 */
BulletHeightfieldShape::
BulletHeightfieldShape(const PNMImage &image, PN_stdfloat max_height, BulletUpAxis up) {

  _num_rows = image.get_x_size();
  _num_cols = image.get_y_size();

  _data = new float[_num_rows * _num_cols];

  for (int row=0; row < _num_rows; row++) {
    for (int column=0; column < _num_cols; column++) {
      _data[_num_cols * row + column] =
        max_height * image.get_bright(column, _num_cols - row - 1);
    }
  }

  _shape = new btHeightfieldTerrainShape(_num_rows,
                                         _num_cols,
                                         _data,
                                         max_height,
                                         up,
                                         true, false);
  _shape->setUserPointer(this);
}

/**

 */
btCollisionShape *BulletHeightfieldShape::
ptr() const {

  return _shape;
}

/**

 */
void BulletHeightfieldShape::
set_use_diamond_subdivision(bool flag) {

  return _shape->setUseDiamondSubdivision(flag);
}
