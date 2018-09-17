/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletMultiSphereShape.cxx
 * @author enn0x
 * @date 2012-01-05
 */

#include "bulletMultiSphereShape.h"

#include "bulletWorld.h"

#include "geomVertexReader.h"

TypeHandle BulletMultiSphereShape::_type_handle;

/**
 *
 */
BulletMultiSphereShape::
BulletMultiSphereShape(const PTA_LVecBase3 &points, const PTA_stdfloat &radii) {

  int num_spheres = std::min(points.size(), radii.size());

  // Convert points
  btVector3 *bt_points = new btVector3[num_spheres];
  for (int i=0; i<num_spheres; i++) {
    bt_points[i] = LVecBase3_to_btVector3(points[i]);
  }

  // Convert radii
  btScalar *bt_radii = new btScalar[num_spheres];
  for (int j=0; j<num_spheres; j++) {
    bt_radii[j] = (PN_stdfloat)radii[j];
  }

  // Create shape
  _shape = new btMultiSphereShape(bt_points, bt_radii, num_spheres);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletMultiSphereShape::
BulletMultiSphereShape(const BulletMultiSphereShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _shape = copy._shape;
}

/**
 *
 */
void BulletMultiSphereShape::
operator = (const BulletMultiSphereShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _shape = copy._shape;
}

/**
 *
 */
btCollisionShape *BulletMultiSphereShape::
ptr() const {

  return _shape;
}

/**
 *
 */
int BulletMultiSphereShape::
get_sphere_count() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _shape->getSphereCount();
}

/**
 *
 */
LPoint3 BulletMultiSphereShape::
get_sphere_pos(int index) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(index >=0 && index <_shape->getSphereCount(), LPoint3::zero());
  return btVector3_to_LPoint3(_shape->getSpherePosition(index));
}

/**
 *
 */
PN_stdfloat BulletMultiSphereShape::
get_sphere_radius(int index) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(index >=0 && index <_shape->getSphereCount(), 0.0);
  return (PN_stdfloat)_shape->getSphereRadius(index);
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletMultiSphereShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletMultiSphereShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);
  dg.add_stdfloat(get_margin());

  // parameters to serialize: sphere count, points, radii
  dg.add_int32(get_sphere_count());
  for (int i = 0; i < get_sphere_count(); ++i){
    get_sphere_pos(i).write_datagram(dg);
  }

  for (int i = 0; i < get_sphere_count(); ++i){
    dg.add_stdfloat(get_sphere_radius(i));
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletMultiSphereShape::
make_from_bam(const FactoryParams &params) {
  // create a default BulletMultiSphereShape
  BulletMultiSphereShape *param = new BulletMultiSphereShape;
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
void BulletMultiSphereShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);
  nassertv(_shape == nullptr);

  PN_stdfloat margin = scan.get_stdfloat();

  // parameters to serialize: sphere count, points, radii
  int sphereCount = scan.get_int32();
  btVector3 *positions = new btVector3[sphereCount];
  for (int i = 0; i < sphereCount; ++i){
    LVector3 pos;
    pos.read_datagram(scan);
    positions[i] = LVecBase3_to_btVector3(pos);
  }

  btScalar *radii = new btScalar[sphereCount];
  for (int i = 0; i < sphereCount; ++i){
    radii[i] = scan.get_stdfloat();
  }

  _shape = new btMultiSphereShape(positions, radii, sphereCount);
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
