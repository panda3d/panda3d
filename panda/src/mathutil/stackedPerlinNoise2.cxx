// Filename: stackedPerlinNoise2.cxx
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

#include "stackedPerlinNoise2.h"

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::Constructor
//       Access: Published
//  Description: Creates num_levels nested PerlinNoise2 objects.  Each
//               stacked Perlin object will have a scale of 1 /
//               scale_factor times the previous object (so that it is
//               higher-frequency, if scale_factor > 1), and an
//               amplitude of amp_scale times the previous object (so
//               that it is less important, if amp_scale < 1).
////////////////////////////////////////////////////////////////////
StackedPerlinNoise2::
StackedPerlinNoise2(double sx, double sy, int num_levels,
                    double scale_factor, double amp_scale,
                    int table_size, unsigned long seed) :
  _amp_scale(amp_scale) 
{
  _noises.reserve(num_levels);
  for (int i = 0; i < num_levels; ++i) {
    PerlinNoise2 noise(sx, sy, table_size, seed);
    _noises.push_back(noise);
    seed = noise.get_seed();
    sx /= scale_factor;
    sy /= scale_factor;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
double StackedPerlinNoise2::
noise(const LVecBase2d &value) {
  double result = 0.0f;
  double amp = 1.0f;

  Noises::iterator ni;
  for (ni = _noises.begin(); ni != _noises.end(); ++ni) {
    result += (*ni).noise(value) * amp;
    amp *= _amp_scale;
  }

  return result;
}
