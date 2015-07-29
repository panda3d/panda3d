// Filename: bulletConvexHullShape.cxx
// Created by:  enn0x (26Jan10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "bulletConvexHullShape.h"

#include "nodePathCollection.h"
#include "geomNode.h"
#include "geomVertexReader.h"

TypeHandle BulletConvexHullShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletConvexHullShape::
BulletConvexHullShape() {

  _shape = new btConvexHullShape(NULL, 0);
  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletConvexHullShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::add_point
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConvexHullShape::
add_point(const LPoint3 &p) {

  _shape->addPoint(LVecBase3_to_btVector3(p));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConvexHullShape::
add_array(const PTA_LVecBase3 &points) {

  _shape = new btConvexHullShape(NULL, 0);
  _shape->setUserPointer(this);

  PTA_LVecBase3::const_iterator it;

#if BT_BULLET_VERSION >= 282
  for (it = points.begin(); it != points.end(); ++it) {
    _shape->addPoint(LVecBase3_to_btVector3(*it), false);
  }
  _shape->recalcLocalAabb();
#else
  for (it = points.begin(); it != points.end(); ++it) {
    _shape->addPoint(LVecBase3_to_btVector3(*it));
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletConvexHullShape::
add_geom(const Geom *geom, const TransformState *ts) {

  nassertv(geom);
  nassertv(ts);

  LMatrix4 m = ts->get_mat();

  // Collect points
  pvector<LPoint3> points;

  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  GeomVertexReader reader = GeomVertexReader(vdata, InternalName::get_vertex());

  while (!reader.is_at_end()) {
    points.push_back(m.xform_point(reader.get_data3()));
  }

  // Create shape
  _shape = new btConvexHullShape(NULL, 0);
  _shape->setUserPointer(this);

  pvector<LPoint3>::const_iterator it;

#if BT_BULLET_VERSION >= 282
  for (it = points.begin(); it != points.end(); ++it) {
    LVecBase3 v = *it;
    _shape->addPoint(LVecBase3_to_btVector3(*it), false);
  }
  _shape->recalcLocalAabb();
#else
  for (it = points.begin(); it != points.end(); ++it) {
    LVecBase3 v = *it;
    _shape->addPoint(LVecBase3_to_btVector3(*it));
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               BulletShape.
////////////////////////////////////////////////////////////////////
void BulletConvexHullShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void BulletConvexHullShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_stdfloat(get_margin());

  unsigned int num_points = _shape->getNumPoints();
  dg.add_uint32(num_points);

  const btVector3 *points = _shape->getUnscaledPoints();

  for (unsigned int i = 0; i < num_points; ++i) {
    LVecBase3 point = btVector3_to_LVecBase3(points[i]);
    point.write_datagram(dg);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type BulletShape is encountered
//               in the Bam file.  It should create the BulletShape
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *BulletConvexHullShape::
make_from_bam(const FactoryParams &params) {
  BulletConvexHullShape *param = new BulletConvexHullShape;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new BulletShape.
////////////////////////////////////////////////////////////////////
void BulletConvexHullShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  PN_stdfloat margin = scan.get_stdfloat();
  unsigned int num_points = scan.get_uint32();

#if BT_BULLET_VERSION >= 282
  for (unsigned int i = 0; i < num_points; ++i) {
    LVecBase3 point;
    point.read_datagram(scan);
    _shape->addPoint(LVecBase3_to_btVector3(point), false);
  }
  _shape->recalcLocalAabb();
#else
  for (unsigned int i = 0; i < num_points; ++i) {
    LVecBase3 point;
    point.read_datagram(scan);
    _shape->addPoint(LVecBase3_to_btVector3(point));
  }
#endif
}
