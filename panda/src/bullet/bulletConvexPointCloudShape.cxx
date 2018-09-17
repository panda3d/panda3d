/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConvexPointCloudShape.cxx
 * @author enn0x
 * @date 2010-01-30
 */

#include "bulletConvexPointCloudShape.h"

#include "bulletWorld.h"

#include "bullet_utils.h"

#include "geomVertexReader.h"

TypeHandle BulletConvexPointCloudShape::_type_handle;

/**
 *
 */
BulletConvexPointCloudShape::
BulletConvexPointCloudShape(const PTA_LVecBase3 &points, LVecBase3 scale) :
  _scale(scale) {

  btVector3 btScale = LVecBase3_to_btVector3(scale);

  // Convert points
  btVector3 *btPoints = new btVector3[points.size()];

  int i = 0;
  PTA_LVecBase3::const_iterator it;
  for (it=points.begin(); it!=points.end(); it++) {
    btPoints[i] = LVecBase3_to_btVector3(*it);
    i++;
  }

  // Create shape
  _shape = new btConvexPointCloudShape(btPoints, points.size(), btScale);
  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletConvexPointCloudShape::
ptr() const {

  return _shape;
}

/**
 *
 */
BulletConvexPointCloudShape::
BulletConvexPointCloudShape(const Geom *geom, LVecBase3 scale) {

  btVector3 btScale = LVecBase3_to_btVector3(scale);
  _scale = scale;

  // Collect points
  pvector<LPoint3> points;

  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  GeomVertexReader reader = GeomVertexReader(vdata, InternalName::get_vertex());

  while (!reader.is_at_end()) {
    points.push_back(reader.get_data3());
  }

  // Convert points
  btVector3 *btPoints = new btVector3[points.size()];

  int i = 0;
  pvector<LPoint3>::const_iterator it;
  for (it=points.begin(); it!=points.end(); it++) {
    btPoints[i] = LVecBase3_to_btVector3(*it);
    i++;
  }

  // Create
  _shape = new btConvexPointCloudShape(btPoints, points.size(), btScale);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletConvexPointCloudShape::
BulletConvexPointCloudShape(const BulletConvexPointCloudShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _scale = copy._scale;

  btVector3 *btPoints = copy._shape->getUnscaledPoints();
  int numPoints = copy._shape->getNumPoints();
  btVector3 btScale = LVecBase3_to_btVector3(_scale);

  _shape = new btConvexPointCloudShape(btPoints, numPoints, btScale);
  _shape->setUserPointer(this);
}

/**
 *
 */
int BulletConvexPointCloudShape::
get_num_points() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _shape->getNumPoints();
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletConvexPointCloudShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletConvexPointCloudShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);

  // parameters to serialize: num points, points, scale
  _scale.write_datagram(dg);

  dg.add_int32(get_num_points());
  for (int i = 0; i < get_num_points(); ++i){
    btVector3_to_LVector3(_shape->getUnscaledPoints()[i]).write_datagram(dg);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletConvexPointCloudShape::
make_from_bam(const FactoryParams &params) {
  // create a default BulletConvexPointCloudShape
  BulletConvexPointCloudShape *param = new BulletConvexPointCloudShape;
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
void BulletConvexPointCloudShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);

  // parameters to serialize: num points, points, scale
  _scale.read_datagram(scan);

  unsigned int num_points = scan.get_uint32();

  btVector3 *btPoints = new btVector3[num_points];
  for (unsigned int i = 0; i < num_points; ++i) {
    LPoint3 point;
    point.read_datagram(scan);
    btPoints[i] = LVecBase3_to_btVector3(point);
  }

  // Create shape
  _shape = new btConvexPointCloudShape(btPoints, num_points, LVecBase3_to_btVector3(_scale));
  _shape->setUserPointer(this);
}
