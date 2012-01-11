// Filename: bulletBoxShape.cxx
// Created by:  enn0x (24Jan10)
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

#include "bulletBoxShape.h"
#include "bullet_utils.h"

TypeHandle BulletBoxShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletBoxShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletBoxShape::
BulletBoxShape(const LVecBase3 &halfExtents) {

  btVector3 btHalfExtents = LVecBase3_to_btVector3(halfExtents);

  _shape = new btBoxShape(btHalfExtents);
  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBoxShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletBoxShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBoxShape::get_half_extents_without_margin
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVecBase3 BulletBoxShape::
get_half_extents_without_margin() const {

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithoutMargin());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBoxShape::get_half_extents_with_margin
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVecBase3 BulletBoxShape::
get_half_extents_with_margin() const {

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithMargin());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBoxShape::make_from_solid
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletBoxShape *BulletBoxShape::
make_from_solid(const CollisionBox *solid) {

  LPoint3 p0 = solid->get_min();
  LPoint3 p1 = solid->get_max();

  LVecBase3 extents(p1.get_x() - p0.get_x() / 2.0,
                     p1.get_y() - p0.get_y() / 2.0,
                     p1.get_z() - p0.get_z() / 2.0);

  return new BulletBoxShape(extents);
}

