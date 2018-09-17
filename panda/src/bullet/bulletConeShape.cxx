/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConeShape.cxx
 * @author enn0x
 * @date 2010-01-24
 */

#include "bulletConeShape.h"

#include "config_bullet.h"

#include "bulletWorld.h"

TypeHandle BulletConeShape::_type_handle;

/**
 *
 */
BulletConeShape::
BulletConeShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up) :
  _radius(radius),
  _height(height),
  _up(up) {

  switch (up) {
  case X_up:
    _shape = new btConeShapeX((btScalar)radius, (btScalar)height);
    break;
  case Y_up:
    _shape = new btConeShape((btScalar)radius, (btScalar)height);
    break;
  case Z_up:
    _shape = new btConeShapeZ((btScalar)radius, (btScalar)height);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << up << std::endl;
    break;
  }

  nassertv(_shape);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletConeShape::
BulletConeShape(const BulletConeShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _up = copy._up;
  _radius = copy._radius;
  _height = copy._height;

  switch (_up) {
  case X_up:
    _shape = new btConeShapeX((btScalar)_radius, (btScalar)_height);
    break;
  case Y_up:
    _shape = new btConeShape((btScalar)_radius, (btScalar)_height);
    break;
  case Z_up:
    _shape = new btConeShapeZ((btScalar)_radius, (btScalar)_height);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << _up << std::endl;
    break;
  }

  nassertv(_shape);
  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletConeShape::
ptr() const {

  return _shape;
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletConeShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletConeShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);
  dg.add_stdfloat(get_margin());

  // parameters to serialize: radius, height, upIndex
  dg.add_stdfloat(_radius);
  dg.add_stdfloat(_height);
  dg.add_int8((int8_t)_up);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletConeShape::
make_from_bam(const FactoryParams &params) {
  // create a default BulletConeShape
  BulletConeShape *param = new BulletConeShape;
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
void BulletConeShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);
  nassertv(_shape == nullptr);

  PN_stdfloat margin = scan.get_stdfloat();

  // parameters to serialize: radius, height, up
  _radius = scan.get_stdfloat();
  _height = scan.get_stdfloat();
  _up = (BulletUpAxis) scan.get_int8();

  switch (_up) {
  case 0:
    _shape = new btConeShapeX((btScalar)_radius, (btScalar)_height);
    break;
  case 1:
    _shape = new btConeShape((btScalar)_radius, (btScalar)_height);
    break;
  case 2:
    _shape = new btConeShapeZ((btScalar)_radius, (btScalar)_height);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << _up << std::endl;
    break;
  }

  nassertv(_shape);
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
