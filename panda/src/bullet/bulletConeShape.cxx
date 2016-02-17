/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConeShape.cxx
 * @author enn0x
 * @date 2010-01-24
 */

#include "bulletConeShape.h"

TypeHandle BulletConeShape::_type_handle;

/**
 *
 */
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

/**
 *
 */
btCollisionShape *BulletConeShape::
ptr() const {

  return _shape;
}
