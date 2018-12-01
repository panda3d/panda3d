/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSphereShape.cxx
 * @author enn0x
 * @date 2010-01-23
 */

#include "bulletSphereShape.h"

#include "bulletWorld.h"

TypeHandle BulletSphereShape::_type_handle;

/**
 *
 */
BulletSphereShape::
BulletSphereShape(PN_stdfloat radius) : _radius(radius) {

  _shape = new btSphereShape(radius);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletSphereShape::
BulletSphereShape(const BulletSphereShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _radius = copy._radius;

  _shape = new btSphereShape(_radius);
  _shape->setUserPointer(this);
}


/**
 *
 */
btCollisionShape *BulletSphereShape::
ptr() const {

  return _shape;
}

/**
 *
 */
BulletSphereShape *BulletSphereShape::
make_from_solid(const CollisionSphere *solid) {

  return new BulletSphereShape(solid->get_radius());
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletSphereShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletSphereShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);

  dg.add_stdfloat(get_margin());
  dg.add_stdfloat(_radius);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletSphereShape::
make_from_bam(const FactoryParams &params) {
  BulletSphereShape *param = new BulletSphereShape;
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
void BulletSphereShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);
  nassertv(_shape == nullptr);

  PN_stdfloat margin = scan.get_stdfloat();
  _radius = scan.get_stdfloat();

  _shape = new btSphereShape(_radius);
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
