/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletBoxShape.I
 * @author enn0x
 * @date 2010-01-24
 */

/**
 * Only used by make_from_bam.
 */
INLINE BulletBoxShape::
BulletBoxShape() :
  _shape(nullptr),
  _half_extents(LVecBase3::zero()) {
}

/**
 *
 */
INLINE BulletBoxShape::
~BulletBoxShape() {

  delete _shape;
}
