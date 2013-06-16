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
  for (it=points.begin(); it!=points.end(); it++) {
    LVecBase3 v = *it;
    _shape->addPoint(LVecBase3_to_btVector3(v));
  }
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
  for (it=points.begin(); it!=points.end(); it++) {
    _shape->addPoint(LVecBase3_to_btVector3(*it));
  }
}

