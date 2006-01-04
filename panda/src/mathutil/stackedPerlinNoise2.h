// Filename: stackedPerlinNoise2.h
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

#ifndef STACKEDPERLINNOISE2_H
#define STACKEDPERLINNOISE2_H

#include "pandabase.h"
#include "perlinNoise2.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : StackedPerlinNoise2
// Description : Implements a multi-layer PerlinNoise, with one or
//               more high-frequency noise functions added to a
//               lower-frequency base noise function.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA StackedPerlinNoise2 {
PUBLISHED:
  INLINE StackedPerlinNoise2();
  StackedPerlinNoise2(double sx, double sy, int num_levels = 2,
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

