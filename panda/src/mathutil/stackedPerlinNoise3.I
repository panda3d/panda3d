// Filename: stackedPerlinNoise3.I
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


////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise3::Default Constructor
//       Access: Published
//  Description: Creates a StackedPerlinNoise3 object with no levels.
//               You should call add_level() to add each level by
//               hand.
////////////////////////////////////////////////////////////////////
INLINE StackedPerlinNoise3::
StackedPerlinNoise3() {
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise3::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE double StackedPerlinNoise3::
noise(double x, double y, double z) {
  return noise(LVecBase3d(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise3::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE float StackedPerlinNoise3::
noise(const LVecBase3f &value) {
  return (float)noise(value[0], value[1], value[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise3::operator ()
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE double StackedPerlinNoise3::
operator ()(double x, double y, double z) {
  return noise(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise3::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE float StackedPerlinNoise3::
operator ()(const LVecBase3f &value) {
  return noise(value);
}

////////////////////////////////////////////////////////////////////
//     Function: StackedPerlinNoise3::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE double StackedPerlinNoise3::
operator ()(const LVecBase3d &value) {
  return noise(value);
}
