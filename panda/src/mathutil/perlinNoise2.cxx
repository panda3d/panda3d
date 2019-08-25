/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file perlinNoise2.cxx
 * @author drose
 * @date 2005-10-05
 */

#include "perlinNoise2.h"
#include "cmath.h"

/**
 * Returns the noise function of the three inputs.
 */
double PerlinNoise2::
noise(const LVecBase2d &value) const {
  // If this triggers, you passed in 0 for table_size.
  nassertr(!_index.empty(), make_nan(0.0));

  // Convert the vector to our local coordinate space.
  LVecBase2d vec = _input_xform.xform_point(value);

  double x = vec[0];
  double y = vec[1];

  // Find unit square that contains point.
  double xf = cfloor(x);
  double yf = cfloor(y);

  int X = ((int)xf) & _table_size_mask;
  int Y = ((int)yf) & _table_size_mask;

  // Find relative x,y of point in square.
  x -= xf;
  y -= yf;

  // Compute fade curves for each of x,y.
  double u = fade(x);
  double v = fade(y);

  // Hash coordinates of the 4 square corners (A, B, A + 1, and B + 1)
  nassertr(X >= 0 && X + 1 < _index.size(), make_nan(0.0));
  int A = _index[X] + Y;
  int B = _index[X + 1] + Y;

  nassertr(A >= 0 && A + 1 < _index.size(), make_nan(0.0));
  nassertr(B >= 0 && B + 1 < _index.size(), make_nan(0.0));

  // and add blended results from 4 corners of square.
  double result =
    lerp(v, lerp(u, grad(_index[A], x, y),
                 grad(_index[B], x - 1, y)),
         lerp(u, grad(_index[A + 1], x, y - 1),
              grad(_index[B + 1], x - 1, y - 1)));

  return result;
}

/**
 * Come up with a random rotation to apply to the input coordinates.  This
 * will reduce the problem of the singularities on the axes, by sending the
 * axes in some crazy direction.
 */
void PerlinNoise2::
init_unscaled_xform() {
  double rot = _randomizer.random_real(360.0f);
  _unscaled_xform = LMatrix3d::rotate_mat(rot);

  // And come up with a random translation too, just so the singularity at (0,
  // 0) is also unpredicatable.
  _unscaled_xform.set_row(2, LVecBase2d(_randomizer.random_real_unit(),
                                        _randomizer.random_real_unit()));
}
