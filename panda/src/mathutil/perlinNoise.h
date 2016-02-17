/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file perlinNoise.h
 * @author drose
 * @date 2005-10-05
 */

#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include "pandabase.h"
#include "pvector.h"
#include "vector_int.h"
#include "luse.h"
#include "randomizer.h"

/**
 * This is the base class for PerlinNoise2 and PerlinNoise3, different
 * dimensions of Perlin noise implementation.  The base class just collects
 * the common functionality.
 */
class EXPCL_PANDA_MATHUTIL PerlinNoise {
protected:
  PerlinNoise(int table_size, unsigned long seed);
  PerlinNoise(const PerlinNoise &copy);
  void operator = (const PerlinNoise &copy);

  INLINE static double fade(double t);
  INLINE static double lerp(double t, double a, double b);

PUBLISHED:
  INLINE unsigned long get_seed();

protected:
  int _table_size;
  int _table_size_mask;

  Randomizer _randomizer;

  typedef vector_int Index;
  Index _index;
};

#include "perlinNoise.I"

#endif
