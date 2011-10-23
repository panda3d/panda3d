// Filename: bulletPlaneShape.cxx
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

#include "bulletPlaneShape.h"

TypeHandle BulletPlaneShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletPlaneShape::
BulletPlaneShape(const LVector3 &normal, PN_stdfloat constant) {

  btVector3 btNormal = LVecBase3_to_btVector3(normal);

  _shape = new btStaticPlaneShape(btNormal, constant);
  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletPlaneShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::make_from_solid
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletPlaneShape *BulletPlaneShape::
make_from_solid(const CollisionPlane *solid) {

  LVector3 normal = solid->get_normal();
  PN_stdfloat constant = solid->dist_to_plane(LPoint3(0, 0, 0));

  return new BulletPlaneShape(normal, constant);
}

