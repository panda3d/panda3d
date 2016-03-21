/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCylinderShape.cxx
 * @author enn0x
 * @date 2010-02-17
 */

#include "bulletCylinderShape.h"

TypeHandle BulletCylinderShape::_type_handle;

/**
 *
 */
BulletCylinderShape::
BulletCylinderShape(const LVector3 &half_extents, BulletUpAxis up) {

  btVector3 btHalfExtents = LVecBase3_to_btVector3(half_extents);

  switch (up) {
  case X_up:
    _shape = new btCylinderShapeX(btHalfExtents);
    break;
  case Y_up:
    _shape = new btCylinderShape(btHalfExtents);
    break;
  case Z_up:
    _shape = new btCylinderShapeZ(btHalfExtents);
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
BulletCylinderShape::
BulletCylinderShape(PN_stdfloat radius, PN_stdfloat height, BulletUpAxis up) {

  switch (up) {
  case X_up:
    _shape = new btCylinderShapeX(btVector3(0.5 * height, radius, radius));
    break;
  case Y_up:
    _shape = new btCylinderShape(btVector3(radius, 0.5 * height, radius));
    break;
  case Z_up:
    _shape = new btCylinderShapeZ(btVector3(radius, radius, 0.5 * height));
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
btCollisionShape *BulletCylinderShape::
ptr() const {

  return _shape;
}
