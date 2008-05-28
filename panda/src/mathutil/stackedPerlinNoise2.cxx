// Filename: stackedPerlinNoise2.cxx
// Created by:  drose (05Oct05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::Copy Constructor
//       Access: Published
//  Description: Creates an exact duplicate of the existing
//               StackedPerlinNoise2 object, including the random
//               seed.
////////////////////////////////////////////////////////////////////
StackedPerlinNoise2::
StackedPerlinNoise2(const StackedPerlinNoise2 &copy) :
  _noises(copy._noises)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::Copy Assignment Operator
//       Access: Published
//  Description: Creates an exact duplicate of the existing
//               StackedPerlinNoise2 object, including the random
//               seed.
////////////////////////////////////////////////////////////////////
void StackedPerlinNoise2::
operator = (const StackedPerlinNoise2 &copy) {
  _noises = copy._noises;
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::add_level
//       Access: Published
//  Description: Adds an arbitrary PerlinNoise2 object, and an
//               associated amplitude, to the stack.
////////////////////////////////////////////////////////////////////
void StackedPerlinNoise2::
add_level(const PerlinNoise2 &level, double amp) {
  _noises.push_back(Noise());
  Noise &n = _noises.back();
  n._noise = level;
  n._amp = amp;
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::clear
//       Access: Published
//  Description: Removes all levels from the stack.  You must call
//               add_level() again to restore them.
////////////////////////////////////////////////////////////////////
void StackedPerlinNoise2::
clear() {
  _noises.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
double StackedPerlinNoise2::
noise(const LVecBase2d &value) {
  double result = 0.0;

  Noises::iterator ni;
  for (ni = _noises.begin(); ni != _noises.end(); ++ni) {
    result += (*ni)._noise(value) * (*ni)._amp;
  }

  return result;
}
