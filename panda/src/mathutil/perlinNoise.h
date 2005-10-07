// Filename: perlinNoise.h
// Created by:  drose (05Oct05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include "pandabase.h"
#include "pvector.h"
#include "luse.h"
#include "mersenne.h"

////////////////////////////////////////////////////////////////////
//       Class : PerlinNoise
// Description : This is the base class for PerlinNoise2 and
//               PerlinNoise3, different dimensions of Perlin noise
//               implementation.  The base class just collects the
//               common functionality.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PerlinNoise {
protected:
  PerlinNoise(int table_size, unsigned long seed);

  INLINE static double fade(double t);
  INLINE static double lerp(double t, double a, double b);

  INLINE int random_int(int range);
  INLINE double random_real(double range);
  INLINE double random_real_unit();

  INLINE static unsigned long get_next_seed();

protected:
  int _table_size;

  Mersenne _mersenne;
  static Mersenne _next_seed;
  static bool _got_first_seed;

  typedef pvector<int> Index;
  Index _index;
};

#include "perlinNoise.I"

#endif
