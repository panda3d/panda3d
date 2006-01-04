// Filename: stackedPerlinNoise2.I
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


////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::Default Constructor
//       Access: Published
//  Description: Creates a StackedPerlinNoise2 object with no levels.
//               You should call add_level() to add each level by
//               hand.
////////////////////////////////////////////////////////////////////
INLINE StackedPerlinNoise2::
StackedPerlinNoise2() {
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE double StackedPerlinNoise2::
noise(double x, double y) {
  return noise(LVecBase2d(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE float StackedPerlinNoise2::
noise(const LVecBase2f &value) {
  return (float)noise(value[0], value[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::operator ()
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE double StackedPerlinNoise2::
operator ()(double x, double y) {
  return noise(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE float StackedPerlinNoise2::
operator ()(const LVecBase2f &value) {
  return noise(value);
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE double StackedPerlinNoise2::
operator ()(const LVecBase2d &value) {
  return noise(value);
}
