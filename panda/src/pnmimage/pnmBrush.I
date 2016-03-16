/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmBrush.I
 * @author drose
 * @date 2007-02-01
 */

/**
 *
 */
INLINE PNMBrush::
PNMBrush(float xc, float yc) : _xc(xc), _yc(yc) {
}

/**
 * Returns the coordinates of the brush's center pixel.  For a one-pixel
 * brush, this will be (0.5, 0.5); for a centered two-pixel brush, this will
 * be (1.0, 1.0); for a centered three-pixel brush, this will be (1.5, 1.5);
 * and so on.
 */
INLINE float PNMBrush::
get_xc() const {
  return _xc;
}

/**
 * Returns the coordinates of the brush's center pixel.  For a one-pixel
 * brush, this will be (0.5, 0.5); for a centered two-pixel brush, this will
 * be (1.0, 1.0); for a centered three-pixel brush, this will be (1.5, 1.5);
 * and so on.
 */
INLINE float PNMBrush::
get_yc() const {
  return _yc;
}
