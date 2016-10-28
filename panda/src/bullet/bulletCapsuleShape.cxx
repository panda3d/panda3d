/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCapsuleShape.cxx
 * @author enn0x
 * @date 2010-01-27
 */

#include "bulletCapsuleShape.h"

TypeHandle BulletCapsuleShape::_type_handle;

/**
 *
 */
BulletCapsuleShape::
BulletCapsuleShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up):
  _radius(radius), _height(height){

  switch (up) {
  case X_up:
    _shape = new btCapsuleShapeX(radius, height);
    break;
  case Y_up:
    _shape = new btCapsuleShape(radius, height);
    break;
  case Z_up:
    _shape = new btCapsuleShapeZ(radius, height);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << up << endl;
    break;
  }

  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletCapsuleShape::
ptr() const {

  return _shape;
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletCapsuleShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletCapsuleShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);

  // parameters to serialize: radius, height, up
  dg.add_stdfloat(_radius);
  dg.add_stdfloat(_height);
  dg.add_int8((int8_t)_shape->getUpAxis());
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletCapsuleShape::
make_from_bam(const FactoryParams &params) {
  // create a default BulletCapsuleShape
  BulletCapsuleShape *param = new BulletCapsuleShape;
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
void BulletCapsuleShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  nassertv(_shape == NULL);
  BulletShape::fillin(scan, manager);

  // parameters to serialize: radius, height, up
  _radius = scan.get_stdfloat();
  _height = scan.get_stdfloat();
  int up = (int) scan.get_int8();

  switch (up) {
  case X_up:
    _shape = new btCapsuleShapeX(_radius, _height);
    break;
  case Y_up:
    _shape = new btCapsuleShape(_radius, _height);
    break;
  case Z_up:
    _shape = new btCapsuleShapeZ(_radius, _height);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << up << endl;
    break;
  }

  _shape->setUserPointer(this);
}
