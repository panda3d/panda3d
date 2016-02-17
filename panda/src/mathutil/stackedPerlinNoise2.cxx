/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stackedPerlinNoise2.cxx
 * @author drose
 * @date 2005-10-05
 */

#include "stackedPerlinNoise2.h"

/**
 * Creates num_levels nested PerlinNoise2 objects.  Each stacked Perlin object
 * will have a scale of 1 scale_factor times the previous object (so that it is
 * higher-frequency, if scale_factor > 1), and an amplitude of amp_scale times
 * the previous object (so that it is less important, if amp_scale < 1).
 */
StackedPerlinNoise2::
StackedPerlinNoise2(double sx, double sy, int num_levels,
                    double scale_factor, double amp_scale,
                    int table_size, unsigned long seed) {
  _noises.reserve(num_levels);
  double amp = 1.0;
  for (int i = 0; i < num_levels; ++i) {
    PerlinNoise2 noise(sx, sy, table_size, seed);
    add_level(noise, amp);

    seed = noise.get_seed();
    amp *= amp_scale;
    sx /= scale_factor;
    sy /= scale_factor;
  }
}

/**
 * Creates an exact duplicate of the existing StackedPerlinNoise2 object,
 * including the random seed.
 */
StackedPerlinNoise2::
StackedPerlinNoise2(const StackedPerlinNoise2 &copy) :
  _noises(copy._noises)
{
}

/**
 * Creates an exact duplicate of the existing StackedPerlinNoise2 object,
 * including the random seed.
 */
void StackedPerlinNoise2::
operator = (const StackedPerlinNoise2 &copy) {
  _noises = copy._noises;
}

/**
 * Adds an arbitrary PerlinNoise2 object, and an associated amplitude, to the
 * stack.
 */
void StackedPerlinNoise2::
add_level(const PerlinNoise2 &level, double amp) {
  _noises.push_back(Noise());
  Noise &n = _noises.back();
  n._noise = level;
  n._amp = amp;
}

/**
 * Removes all levels from the stack.  You must call add_level() again to
 * restore them.
 */
void StackedPerlinNoise2::
clear() {
  _noises.clear();
}

/**
 * Returns the noise function of the three inputs.
 */
double StackedPerlinNoise2::
noise(const LVecBase2d &value) {
  double result = 0.0;

  Noises::iterator ni;
  for (ni = _noises.begin(); ni != _noises.end(); ++ni) {
    result += (*ni)._noise(value) * (*ni)._amp;
  }

  return result;
}
