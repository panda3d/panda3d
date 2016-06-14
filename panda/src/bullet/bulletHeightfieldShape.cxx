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
 *   rotating it 90 degrees to the right.
 */
BulletHeightfieldShape::
BulletHeightfieldShape(const PNMImage &image, PN_stdfloat max_height, BulletUpAxis up) {

  _max_height = max_height;
  _y_size = image.get_x_size();
  _x_size = image.get_y_size();
  _data.reserve(_y_size * _x_size);

  PfmFile pfm;
  if (!pfm.load(image)) {
    bullet_cat.error() << "Couldn't convert PNMImage to PfmFile." << endl;
  }
  sample_regular(pfm);

  _shape = new btHeightfieldTerrainShape(_y_size,
                                         _x_size,
                                         _data.data(),
                                         1.0f,
                                         0.0f,
                                         _max_height,
                                         up,
                                         PHY_FLOAT,
                                         false);
  _shape->setUserPointer(this);
}

/**
 * @brief Creates a collision shape suited for terrains from a PfmFile.
 * @details The PfmFile is assumed to have one channel with values in the range
 *          0..1, which translate to 0..max_height elevation.
 * @param STM whether to sample for ShaderTerrainMesh or without interpolation.
 */

BulletHeightfieldShape::
BulletHeightfieldShape(const PfmFile &field, PN_stdfloat max_height, bool STM, BulletUpAxis up) {
  
  LVector3f min_point, max_point;
  field.calc_min_max(min_point, max_point);
  if (min_point.get_x() < 0.0 || max_point.get_x() > 1.0) {
    bullet_cat.error() << "Heightfield PfmFile contains values outside of 0..1 range." << endl;
  }

  _max_height = max_height;

  if (STM) {
    _y_size = field.get_x_size() + 1;
    _x_size = field.get_y_size() + 1;
    _data.reserve(_y_size * _x_size);
    LVector4i entire = LVector4i(0, field.get_x_size()-1, 0, field.get_y_size()-1);
    update_region(entire, field);
  } else {
    _y_size = field.get_x_size();
    _x_size = field.get_y_size();
    _data.reserve(_y_size * _x_size);
    sample_regular(field);
  }

  // using regular non-legacy constructor. Available in Bullet since at least 2.81-rev2613.
  _shape = new btHeightfieldTerrainShape(_y_size,
                                         _x_size,
                                         _data.data(),
                                         1.0f,  // height scale. unused.
                                         0.0f,  // minimum height
                                         _max_height,
                                         up,
                                         PHY_FLOAT,
                                         false);
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
 * @brief This is called by DynamicHeightfield to propagate changes.
 */
void BulletHeightfieldShape::
on_change() {

  update_region(_dynamic_hf->region_corners, *_dynamic_hf);
}

/**
 * @brief Updates the shape with values from a region of the heightfield (described by two corners) using STM sampling.
 */
void BulletHeightfieldShape::
update_region(const LVector4i &corners, const PfmFile &field) {

  PN_stdfloat step_x = 1.0 / (PN_stdfloat)field.get_x_size();
  PN_stdfloat step_y = 1.0 / (PN_stdfloat)field.get_y_size();
  LPoint3f sample;

  for (int row = corners.get_x(); row <= corners.get_y(); row++) {
    for (int column = corners.get_z(); column <= corners.get_w(); column++) {
      if (!field.calc_bilinear_point(sample, row * step_x, column * step_y)) {
        bullet_cat.error() << "Trying to sample unknown point from array." << endl;
      }
      _data[_y_size * (_x_size - 1 - column) + row] = _max_height * sample.get_x();
    }
  }
}

/**
 * @brief Samples the entire PfmFile without interpolation.
 */
void BulletHeightfieldShape::
sample_regular(const PfmFile &pfm) {

  int num_rows = pfm.get_x_size();
  int num_cols = pfm.get_y_size();

  for (int row=0; row < num_rows; row++) {
    for (int column=0; column < num_cols; column++) {
      // Transpose and flip y
      _data[_y_size * column + row] = _max_height * pfm.get_point1(row, num_cols - column - 1);
    }
  }
}
