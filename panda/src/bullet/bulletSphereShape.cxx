// Filename: bulletSphereShape.cxx
// Created by:  enn0x (23Jan10)
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

#include "bulletSphereShape.h"

TypeHandle BulletSphereShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSphereShape::
BulletSphereShape(PN_stdfloat radius) {

  _shape = new btSphereShape(radius);
  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletSphereShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::make_from_solid
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletSphereShape *BulletSphereShape::
make_from_solid(const CollisionSphere *solid) {

  return new BulletSphereShape(solid->get_radius());
}

