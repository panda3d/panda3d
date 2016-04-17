/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCapsuleShape.cxx
 * @author enn0x
 * @date 2010-01-27
 */

#include "bulletCapsuleShape.h"

TypeHandle BulletCapsuleShape::_type_handle;

/**
 *
 */
BulletCapsuleShape::
BulletCapsuleShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up) {

  switch (up) {
  case X_up:
    _shape = new btCapsuleShapeX(radius, height);
    break;
  case Y_up:
    _shape = new btCapsuleShape(radius, height);
    break;
  case Z_up:
    _shape = new btCapsuleShapeZ(radius, height);
    break;
  default:
    bullet_cat.error() << "invalid up-axis:" << up << endl;
    break;
  }

  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletCapsuleShape::
ptr() const {

  return _shape;
}
