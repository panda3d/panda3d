// Filename: bulletConvexPointCloudShape.cxx
// Created by:  enn0x (30Jan10)
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

#include "bulletConvexPointCloudShape.h"

#include "geomVertexReader.h"

TypeHandle BulletConvexPointCloudShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexPointCloudShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletConvexPointCloudShape::
BulletConvexPointCloudShape(const PTA_LVecBase3 &points, LVecBase3 scale) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexPointCloudShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletConvexPointCloudShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexPointCloudShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletConvexPointCloudShape::
BulletConvexPointCloudShape(const Geom *geom, LVecBase3 scale) {

  btVector3 btScale = LVecBase3_to_btVector3(scale);

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

