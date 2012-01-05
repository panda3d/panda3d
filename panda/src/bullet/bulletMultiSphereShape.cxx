// Filename: bulletMultiSphereShape.cxx
// Created by:  enn0x (05Jan12)
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

#include "bulletMultiSphereShape.h"

#include "geomVertexReader.h"

TypeHandle BulletMultiSphereShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletMultiSphereShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletMultiSphereShape::
BulletMultiSphereShape(const PTA_LVecBase3 &points, const PTA_stdfloat &radii) {

  int num_spheres = min(points.size(), radii.size());

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

////////////////////////////////////////////////////////////////////
//     Function: BulletMultiSphereShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletMultiSphereShape::
ptr() const {

  return _shape;
}

