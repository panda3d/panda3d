/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file perlinNoise2.h
 * @author drose
 * @date 2005-10-05
 */

#ifndef PERLINNOISE2_H
#define PERLINNOISE2_H

#include "pandabase.h"
#include "perlinNoise.h"

/**
 * This class provides an implementation of Perlin noise for 2 variables.
 * This code is loosely based on the reference implementation at
 * https://mrl.nyu.edu/~perlin/noise/ .
 */
class EXPCL_PANDA_MATHUTIL PerlinNoise2 : public PerlinNoise {
PUBLISHED:
  INLINE PerlinNoise2();
  INLINE explicit PerlinNoise2(double sx, double sy,
                               int table_size = 256,
                               unsigned long seed = 0);
  INLINE PerlinNoise2(const PerlinNoise2 &copy);
  INLINE void operator = (const PerlinNoise2 &copy);

  INLINE void set_scale(double scale);
  INLINE void set_scale(double sx, double sy);
  INLINE void set_scale(const LVecBase2f &scale);
  INLINE void set_scale(const LVecBase2d &scale);

  INLINE double noise(double x, double y) const;
  INLINE float noise(const LVecBase2f &value) const;
  double noise(const LVecBase2d &value) const;

  INLINE double operator ()(double x, double y) const;
  INLINE float operator ()(const LVecBase2f &value) const;
  INLINE double operator ()(const LVecBase2d &value) const;

private:
  void init_unscaled_xform();
  INLINE static double grad(int hash, double x, double y);

private:
  LMatrix3d _unscaled_xform;
  LMatrix3d _input_xform;
};

#include "perlinNoise2.I"

#endif
