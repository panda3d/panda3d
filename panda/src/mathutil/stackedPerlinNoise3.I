// Filename: stackedPerlinNoise3.I
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
