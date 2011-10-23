// Filename: bulletConeShape.cxx
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

#include "bulletConeShape.h"

TypeHandle BulletConeShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletConeShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletConeShape::
BulletConeShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up) {

  switch (up) {
  case X_up:
    _shape = new btConeShapeX((btScalar)radius, (btScalar)height);
    break;
  case Y_up:
    _shape = new btConeShape((btScalar)radius, (btScalar)height);
    break;
  case Z_up:
    _shape = new btConeShapeZ((btScalar)radius, (btScalar)height);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << up << endl;
    break;
  }

  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConeShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletConeShape::
ptr() const {

  return _shape;
}

