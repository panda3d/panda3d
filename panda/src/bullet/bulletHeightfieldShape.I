/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletHeightfieldShape.I
 * @author enn0x
 * @date 2010-02-05
 */

/**
 * Only used by make_from_bam
 */
INLINE BulletHeightfieldShape::
BulletHeightfieldShape() :
  _num_rows(0),
  _num_cols(0),
  _data(nullptr),
  _shape(nullptr),
  _max_height(0.0),
  _up(Z_up) {
}

/**
 *
 */
INLINE BulletHeightfieldShape::
~BulletHeightfieldShape() {
  delete _shape;
  delete [] _data;
}
