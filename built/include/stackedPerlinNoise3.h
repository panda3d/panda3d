/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stackedPerlinNoise3.h
 * @author drose
 * @date 2005-10-05
 */

#ifndef STACKEDPERLINNOISE3_H
#define STACKEDPERLINNOISE3_H

#include "pandabase.h"
#include "perlinNoise3.h"
#include "epvector.h"

/**
 * Implements a multi-layer PerlinNoise, with one or more high-frequency noise
 * functions added to a lower-frequency base noise function.
 */
class EXPCL_PANDA_MATHUTIL StackedPerlinNoise3 {
PUBLISHED:
  INLINE StackedPerlinNoise3();
  explicit StackedPerlinNoise3(double sx, double sy, double sz, int num_levels = 3,
                               double scale_factor = 4.0f, double amp_scale = 0.5f,
                               int table_size = 256, unsigned long seed = 0);
  StackedPerlinNoise3(const StackedPerlinNoise3 &copy);
  void operator = (const StackedPerlinNoise3 &copy);

  void add_level(const PerlinNoise3 &level, double amp = 1.0);
  void clear();

  INLINE double noise(double x, double y, double z);
  INLINE float noise(const LVecBase3f &value);
  double noise(const LVecBase3d &value);

  INLINE double operator ()(double x, double y, double z);
  INLINE float operator ()(const LVecBase3f &value);
  INLINE double operator ()(const LVecBase3d &value);

private:
  class Noise {
  public:
    PerlinNoise3 _noise;
    double _amp;
  };

  typedef epvector<Noise> Noises;
  Noises _noises;
};

#include "stackedPerlinNoise3.I"

#endif
