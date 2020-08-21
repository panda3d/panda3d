/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCylinderShape.I
 * @author enn0x
 * @date 2010-02-17
 */

/**
 * Only used by make_from_bam.
 */
INLINE BulletCylinderShape::
BulletCylinderShape() :
  _half_extents(LVector3::zero()),
  _shape(nullptr),
  _up(X_up) {
}

/**
 *
 */
INLINE BulletCylinderShape::
~BulletCylinderShape() {

  delete _shape;
}
