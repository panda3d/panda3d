/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletPlaneShape.cxx
 * @author enn0x
 * @date 2010-01-23
 */

#include "bulletPlaneShape.h"

TypeHandle BulletPlaneShape::_type_handle;

/**

 */
BulletPlaneShape::
BulletPlaneShape(const LVector3 &normal, PN_stdfloat constant) {

  btVector3 btNormal = LVecBase3_to_btVector3(normal);

  _shape = new btStaticPlaneShape(btNormal, constant);
  _shape->setUserPointer(this);
}

/**

 */
btCollisionShape *BulletPlaneShape::
ptr() const {

  return _shape;
}

/**

 */
BulletPlaneShape *BulletPlaneShape::
make_from_solid(const CollisionPlane *solid) {

  LVector3 normal = solid->get_normal();
  PN_stdfloat constant = solid->dist_to_plane(LPoint3(0, 0, 0));

  return new BulletPlaneShape(normal, constant);
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletPlaneShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a Bam
 * file.
 */
void BulletPlaneShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_stdfloat(get_margin());
  get_plane_normal().write_datagram(dg);
  dg.add_stdfloat(get_plane_constant());
}

/**
 * This function is called by the BamReader's factory when a new object of type
 * BulletShape is encountered in the Bam file.  It should create the BulletShape
 * and extract its information from the file.
 */
TypedWritable *BulletPlaneShape::
make_from_bam(const FactoryParams &params) {
  BulletPlaneShape *param = new BulletPlaneShape;
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
void BulletPlaneShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  nassertv(_shape == NULL);

  PN_stdfloat margin = scan.get_stdfloat();

  LVector3 normal;
  normal.read_datagram(scan);

  PN_stdfloat constant = scan.get_stdfloat();

  _shape = new btStaticPlaneShape(LVecBase3_to_btVector3(normal), constant);
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
