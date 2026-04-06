/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file perlinNoise.I
 * @author drose
 * @date 2005-10-05
 */

/**
 * Returns a smooth interpolation spline from 0 .. 1 for t.
 */
INLINE double PerlinNoise::
fade(double t) {
  // return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
  return (3.0 - 2.0 * t) * t * t;
}

/**
 * Returns the smoothly lerped value from a to b.
 */
INLINE double PerlinNoise::
lerp(double t, double a, double b) {
  return a + t * (b - a);
}

/**
 * Returns a unique seed value based on the seed value passed to this
 * PerlinNoise object (and on its current state).
 */
INLINE unsigned long PerlinNoise::
get_seed() {
  return _randomizer.get_seed();
}
