/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stackedPerlinNoise2.I
 * @author drose
 * @date 2005-10-05
 */

/**
 * Creates a StackedPerlinNoise2 object with no levels.  You should call
 * add_level() to add each level by hand.
 */
INLINE StackedPerlinNoise2::
StackedPerlinNoise2() {
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE double StackedPerlinNoise2::
noise(double x, double y) {
  return noise(LVecBase2d(x, y));
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE float StackedPerlinNoise2::
noise(const LVecBase2f &value) {
  return (float)noise(value[0], value[1]);
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE double StackedPerlinNoise2::
operator ()(double x, double y) {
  return noise(x, y);
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE float StackedPerlinNoise2::
operator ()(const LVecBase2f &value) {
  return noise(value);
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE double StackedPerlinNoise2::
operator ()(const LVecBase2d &value) {
  return noise(value);
}
