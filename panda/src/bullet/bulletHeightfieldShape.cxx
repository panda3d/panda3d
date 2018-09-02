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

#include "config_bullet.h"

#include "bulletWorld.h"

TypeHandle BulletHeightfieldShape::_type_handle;

/**
 * @brief Creates a collision shape suited for terrains from a rectangular image.
 * @details Stores the image's brightness values in a vector Bullet can use, 
 *   while rotating it 90 degrees to the right.
 */
BulletHeightfieldShape::
BulletHeightfieldShape(const PNMImage &image, PN_stdfloat max_height, BulletUpAxis up) :
  _max_height(max_height), _up(up) {

  _num_rows = image.get_x_size();
  _num_cols = image.get_y_size();

  _data = new btScalar[_num_rows * _num_cols];

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
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _shape->setUseDiamondSubdivision(flag);
}

/**
 * @brief Creates a collision shape suited for terrains from a rectangular texture.
 * @details Alternative constructor intended for use with ShaderTerrainMesh. This will
 *   do bilinear sampling at the corners of all texels. Also works with textures 
 *   that are non-power-of-two and/or rectangular.
 */
BulletHeightfieldShape::
BulletHeightfieldShape(Texture *tex, PN_stdfloat max_height, BulletUpAxis up) :
  _max_height(max_height), _up(up) {

  _num_rows = tex->get_x_size() + 1;
  _num_cols = tex->get_y_size() + 1;
  _data = new btScalar[_num_rows * _num_cols];

  btScalar step_x = 1.0 / (btScalar)tex->get_x_size();
  btScalar step_y = 1.0 / (btScalar)tex->get_y_size();

  PT(TexturePeeker) peeker = tex->peek();
  LColor sample;

  for (int row=0; row < _num_rows; row++) {
    for (int column=0; column < _num_cols; column++) {
      if (!peeker->lookup_bilinear(sample, row * step_x, column * step_y)) {
        bullet_cat.error() << "Could not sample texture." << std::endl;
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

/**
 *
 */
BulletHeightfieldShape::
BulletHeightfieldShape(const BulletHeightfieldShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _num_rows = copy._num_rows;
  _num_cols = copy._num_cols;
  _max_height = copy._max_height;
  _up = copy._up;

  size_t size = (size_t)_num_rows * (size_t)_num_cols;
  _data = new btScalar[size];
  memcpy(_data, copy._data, size * sizeof(btScalar));

  _shape = new btHeightfieldTerrainShape(_num_rows,
                                         _num_cols,
                                         _data,
                                         _max_height,
                                         _up,
                                         true, false);
  _shape->setUserPointer(this);
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletHeightfieldShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletHeightfieldShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);
  dg.add_stdfloat(get_margin());

  // parameters to serialize:_num_rows,_num_cols,_data,max_height,up,
  dg.add_int8((int8_t)_up);
  dg.add_stdfloat(_max_height);
  dg.add_int32(_num_rows);
  dg.add_int32(_num_cols);

  size_t size = (size_t)_num_rows * (size_t)_num_cols;
  for (size_t i = 0; i < size; ++i) {
    dg.add_stdfloat(_data[i]);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletHeightfieldShape::
make_from_bam(const FactoryParams &params) {
  // create a default BulletHeightfieldShape
  BulletHeightfieldShape *param = new BulletHeightfieldShape;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletShape.
 */
void BulletHeightfieldShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);
  nassertv(_shape == nullptr);

  PN_stdfloat margin = scan.get_stdfloat();

  // parameters to serialize: radius, height, up
  _up = (BulletUpAxis) scan.get_int8();
  _max_height = scan.get_stdfloat();
  _num_rows = scan.get_int32();
  _num_cols = scan.get_int32();

  size_t size = (size_t)_num_rows * (size_t)_num_cols;
  delete [] _data;
  _data = new float[size];

  for (size_t i = 0; i < size; ++i) {
    _data[i]  = scan.get_stdfloat();
  }

  _shape = new btHeightfieldTerrainShape(_num_rows,
                                         _num_cols,
                                         _data,
                                         _max_height,
                                         _up,
                                         true, false);
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
