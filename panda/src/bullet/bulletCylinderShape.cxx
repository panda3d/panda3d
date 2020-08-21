/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCylinderShape.cxx
 * @author enn0x
 * @date 2010-02-17
 */

#include "bulletCylinderShape.h"

#include "config_bullet.h"

using std::endl;

TypeHandle BulletCylinderShape::_type_handle;

/**
 *
 */
BulletCylinderShape::
BulletCylinderShape(const LVector3 &half_extents, BulletUpAxis up) :
  _half_extents(half_extents),
  _up(up) {

  btVector3 btHalfExtents = LVecBase3_to_btVector3(half_extents);

  switch (up) {
  case X_up:
    _shape = new btCylinderShapeX(btHalfExtents);
    break;
  case Y_up:
    _shape = new btCylinderShape(btHalfExtents);
    break;
  case Z_up:
    _shape = new btCylinderShapeZ(btHalfExtents);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << up << endl;
    break;
  }

  nassertv(_shape);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletCylinderShape::
BulletCylinderShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up) :
  _up(up) {

  switch (up) {
  case X_up:
    _shape = new btCylinderShapeX(btVector3(0.5 * height, radius, radius));
    _half_extents = btVector3_to_LVector3(btVector3(0.5 * height, radius, radius));
    break;
  case Y_up:
    _shape = new btCylinderShape(btVector3(radius, 0.5 * height, radius));
    _half_extents = btVector3_to_LVector3(btVector3(radius, 0.5 * height, radius));
    break;
  case Z_up:
    _shape = new btCylinderShapeZ(btVector3(radius, radius, 0.5 * height));
    _half_extents = btVector3_to_LVector3(btVector3(radius, radius, 0.5 * height));
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << up << endl;
    break;
  }

  nassertv(_shape);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletCylinderShape::
BulletCylinderShape(const BulletCylinderShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _up = copy._up;
  _half_extents = copy._half_extents;

  btVector3 btHalfExtents = LVecBase3_to_btVector3(_half_extents);

  switch (_up) {
  case X_up:
    _shape = new btCylinderShapeX(btHalfExtents);
    break;
  case Y_up:
    _shape = new btCylinderShape(btHalfExtents);
    break;
  case Z_up:
    _shape = new btCylinderShapeZ(btHalfExtents);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << _up << endl;
    break;
  }

  nassertv(_shape);
  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletCylinderShape::
ptr() const {

  return _shape;
}

/**
 *
 */
PN_stdfloat BulletCylinderShape::
get_radius() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_shape->getRadius();
}

/**
 *
 */
LVecBase3 BulletCylinderShape::
get_half_extents_without_margin() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithoutMargin());
}

/**
 *
 */
LVecBase3 BulletCylinderShape::
get_half_extents_with_margin() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithMargin());
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletCylinderShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletCylinderShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);
  dg.add_stdfloat(get_margin());

  // parameters to serialize: radius, height, up
  _half_extents.write_datagram(dg);
  dg.add_int8((int8_t)_up);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletCylinderShape::
make_from_bam(const FactoryParams &params) {
  // create a default BulletCylinderShape
  BulletCylinderShape *param = new BulletCylinderShape;
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
void BulletCylinderShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);
  nassertv(_shape == nullptr);

  PN_stdfloat margin = scan.get_stdfloat();

  // parameters to serialize: radius, height, up
  _half_extents.read_datagram(scan);
  _up = (BulletUpAxis) scan.get_int8();

  btVector3 btHalfExtents = LVecBase3_to_btVector3(_half_extents);

  switch (_up) {
  case X_up:
    _shape = new btCylinderShapeX(btHalfExtents);
    break;
  case Y_up:
    _shape = new btCylinderShape(btHalfExtents);
    break;
  case Z_up:
    _shape = new btCylinderShapeZ(btHalfExtents);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << _up << endl;
    break;
  }

  nassertv(_shape);
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
