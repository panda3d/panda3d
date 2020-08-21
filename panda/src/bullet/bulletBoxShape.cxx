/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletBoxShape.cxx
 * @author enn0x
 * @date 2010-01-24
 */

#include "bulletBoxShape.h"

#include "bulletWorld.h"

#include "bullet_utils.h"

TypeHandle BulletBoxShape::_type_handle;

/**
 *
 */
BulletBoxShape::
BulletBoxShape(const LVecBase3 &halfExtents) : _half_extents(halfExtents) {

  btVector3 btHalfExtents = LVecBase3_to_btVector3(halfExtents);

  _shape = new btBoxShape(btHalfExtents);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletBoxShape::
BulletBoxShape(const BulletBoxShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _half_extents = copy._half_extents;
  btVector3 btHalfExtents = LVecBase3_to_btVector3(_half_extents);

  _shape = new btBoxShape(btHalfExtents);
  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletBoxShape::
ptr() const {

  return _shape;
}

/**
 *
 */
LVecBase3 BulletBoxShape::
get_half_extents_without_margin() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithoutMargin());
}

/**
 *
 */
LVecBase3 BulletBoxShape::
get_half_extents_with_margin() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithMargin());
}

/**
 *
 */
BulletBoxShape *BulletBoxShape::
make_from_solid(const CollisionBox *solid) {

  LPoint3 p0 = solid->get_min();
  LPoint3 p1 = solid->get_max();

  LVecBase3 extents((p1.get_x() - p0.get_x()) / 2.0,
                    (p1.get_y() - p0.get_y()) / 2.0,
                    (p1.get_z() - p0.get_z()) / 2.0);

  return new BulletBoxShape(extents);
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletBoxShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletBoxShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);
  dg.add_stdfloat(get_margin());
  _half_extents.write_datagram(dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletBoxShape::
make_from_bam(const FactoryParams &params) {
  BulletBoxShape *param = new BulletBoxShape;
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
void BulletBoxShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);
  nassertv(_shape == nullptr);

  PN_stdfloat margin = scan.get_stdfloat();

  _half_extents.read_datagram(scan);

  _shape = new btBoxShape(LVecBase3_to_btVector3(_half_extents));
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
