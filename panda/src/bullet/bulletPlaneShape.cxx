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

#include "bulletWorld.h"

TypeHandle BulletPlaneShape::_type_handle;

/**
 * Creates a plane shape from a plane definition.
 */
BulletPlaneShape::
BulletPlaneShape(LPlane plane) {

  btVector3 btNormal = LVecBase3_to_btVector3(plane.get_normal());

  _shape = new btStaticPlaneShape(btNormal, plane.get_w());
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletPlaneShape::
BulletPlaneShape(const LVector3 &normal, PN_stdfloat constant) {

  btVector3 btNormal = LVecBase3_to_btVector3(normal);

  _shape = new btStaticPlaneShape(btNormal, constant);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletPlaneShape::
BulletPlaneShape(const BulletPlaneShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btVector3 btNormal = copy._shape->getPlaneNormal();
  PN_stdfloat constant = (PN_stdfloat)_shape->getPlaneConstant();

  _shape = new btStaticPlaneShape(btNormal, constant);
  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletPlaneShape::
ptr() const {

  return _shape;
}

/**
 *
 */
LPlane BulletPlaneShape::
get_plane() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btVector3 normal = _shape->getPlaneNormal();
  return LPlane(normal[0], normal[1], normal[2], (PN_stdfloat)_shape->getPlaneConstant());
}

/**
 *
 */
PN_stdfloat BulletPlaneShape::
get_plane_constant() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_shape->getPlaneConstant();
}

/**
 *
 */
LVector3 BulletPlaneShape::
get_plane_normal() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_shape->getPlaneNormal());
}

/**
 *
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
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletPlaneShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);

  dg.add_stdfloat(get_margin());
  get_plane_normal().write_datagram(dg);
  dg.add_stdfloat(get_plane_constant());
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
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
  BulletShape::fillin(scan, manager);
  nassertv(_shape == nullptr);

  PN_stdfloat margin = scan.get_stdfloat();

  LVector3 normal;
  normal.read_datagram(scan);

  PN_stdfloat constant = scan.get_stdfloat();

  _shape = new btStaticPlaneShape(LVecBase3_to_btVector3(normal), constant);
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
