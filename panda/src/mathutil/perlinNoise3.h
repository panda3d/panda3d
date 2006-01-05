// Filename: perlinNoise3.h
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

#ifndef PERLINNOISE3_H
#define PERLINNOISE3_H

#include "pandabase.h"
#include "perlinNoise.h"

////////////////////////////////////////////////////////////////////
//       Class : PerlinNoise3
// Description : This class provides an implementation of Perlin noise
//               for 3 variables.  This code is loosely based on the
//               reference implementation at
//               http://mrl.nyu.edu/~perlin/noise/ .
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PerlinNoise3 : public PerlinNoise {
PUBLISHED:
  INLINE PerlinNoise3();
  INLINE PerlinNoise3(double sx, double sy, double sz,
		      int table_size = 256, unsigned long seed = 0);
  INLINE PerlinNoise3(const PerlinNoise3 &copy);
  INLINE void operator = (const PerlinNoise3 &copy);

  INLINE void set_scale(double scale);
  INLINE void set_scale(double sx, double sy, double sz);
  INLINE void set_scale(const LVecBase3f &scale);
  INLINE void set_scale(const LVecBase3d &scale);

  INLINE double noise(double x, double y, double z);
  INLINE float noise(const LVecBase3f &value);
  double noise(const LVecBase3d &value);

  INLINE double operator ()(double x, double y, double z);
  INLINE float operator ()(const LVecBase3f &value);
  INLINE double operator ()(const LVecBase3d &value);
  
private:
  void init_unscaled_xform();
  INLINE static double grad(int hash, double x, double y, double z);

private:
  LMatrix4d _unscaled_xform;
  LMatrix4d _input_xform;
};

#include "perlinNoise3.I"

#endif

