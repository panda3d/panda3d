/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stackedPerlinNoise2.h
 * @author drose
 * @date 2005-10-05
 */

#ifndef STACKEDPERLINNOISE2_H
#define STACKEDPERLINNOISE2_H

#include "pandabase.h"
#include "perlinNoise2.h"
#include "pvector.h"

/**
 * Implements a multi-layer PerlinNoise, with one or more high-frequency noise
 * functions added to a lower-frequency base noise function.
 */
class EXPCL_PANDA_MATHUTIL StackedPerlinNoise2 {
PUBLISHED:
  INLINE StackedPerlinNoise2();
  explicit StackedPerlinNoise2(double sx, double sy, int num_levels = 2,
                               double scale_factor = 4.0f, double amp_scale = 0.5f,
                               int table_size = 256, unsigned long seed = 0);
  StackedPerlinNoise2(const StackedPerlinNoise2 &copy);
  void operator = (const StackedPerlinNoise2 &copy);

  void add_level(const PerlinNoise2 &level, double amp = 1.0);
  void clear();

  INLINE double noise(double x, double y);
  INLINE float noise(const LVecBase2f &value);
  double noise(const LVecBase2d &value);

  INLINE double operator ()(double x, double y);
  INLINE float operator ()(const LVecBase2f &value);
  INLINE double operator ()(const LVecBase2d &value);

private:
  class Noise {
  public:
    PerlinNoise2 _noise;
    double _amp;
  };

  typedef pvector<Noise> Noises;
  Noises _noises;
};

#include "stackedPerlinNoise2.I"

#endif
