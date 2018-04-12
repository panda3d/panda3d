/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletMinkowskiSumShape.I
 * @author enn0x
 * @date 2010-01-23
 */

/**
 * Only used by make_from_bam.
 */
INLINE BulletMinkowskiSumShape::
BulletMinkowskiSumShape() :
  _shape(nullptr),
  _shape_a(nullptr),
  _shape_b(nullptr) {
}

/**
 *
 */
INLINE BulletMinkowskiSumShape::
~BulletMinkowskiSumShape() {

  delete _shape;
}

/**
 *
 */
INLINE const BulletShape *BulletMinkowskiSumShape::
get_shape_a() const {

  return _shape_a;
}

/**
 *
 */
INLINE const BulletShape *BulletMinkowskiSumShape::
get_shape_b() const {

  return _shape_b;
}
