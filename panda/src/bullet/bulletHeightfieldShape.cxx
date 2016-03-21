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
 * @brief Creates a collision shape suited for terrains from a rectangular image.
 * @details Stores the image's brightness values in a vector Bullet can use, 
 *   while rotating it 90 degrees to the right.
 */
BulletHeightfieldShape::
BulletHeightfieldShape(const PNMImage &image, PN_stdfloat max_height, BulletUpAxis up) {

  _num_rows = image.get_x_size();
  _num_cols = image.get_y_size();

  _data = new float[_num_rows * _num_cols];

  for (int row=0; row < _num_rows; row++) {
    for (int column=0; column < _num_cols; column++) {
      // Transpose
      _data[_num_rows * column + row] =
        // Flip y
        max_height * image.get_bright(row, _num_cols - column - 1);
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
 *
 */
btCollisionShape *BulletHeightfieldShape::
ptr() const {

  return _shape;
}

/**
 *
 */
void BulletHeightfieldShape::
set_use_diamond_subdivision(bool flag) {

  return _shape->setUseDiamondSubdivision(flag);
}

/**
 * @brief Creates a collision shape suited for terrains from a rectangular texture.
 * @details Alternative constructor intended for use with ShaderTerrainMesh. This will
 *   do bilinear sampling at the corners of all texels. Also works with textures 
 *   that are non-power-of-two and/or rectangular.
 */
BulletHeightfieldShape::
BulletHeightfieldShape(Texture *tex, PN_stdfloat max_height, BulletUpAxis up) {

  _num_rows = tex->get_x_size() + 1;
  _num_cols = tex->get_y_size() + 1;
  _data = new float[_num_rows * _num_cols];

  PN_stdfloat step_x = 1.0 / (PN_stdfloat)tex->get_x_size();
  PN_stdfloat step_y = 1.0 / (PN_stdfloat)tex->get_y_size();

  PT(TexturePeeker) peeker = tex->peek();
  LColor sample;

  for (int row=0; row < _num_rows; row++) {
    for (int column=0; column < _num_cols; column++) {
      if (!peeker->lookup_bilinear(sample, row * step_x, column * step_y)) {
        bullet_cat.error() << "Could not sample texture." << endl;
      }
      // Transpose
      _data[_num_rows * column + row] = max_height * sample.get_x();
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