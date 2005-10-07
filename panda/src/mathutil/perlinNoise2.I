// Filename: perlinNoise2.I
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
//     Function: PerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise2::
noise(double x, double y) {
  return noise(LVecBase2d(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE float PerlinNoise2::
noise(const LVecBase2f &value) {
  return (float)noise(value[0], value[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise2::grad
//       Access: Private, Static
//  Description: Returns the dot product of a random gradient vector
//               (determined by the hash code) with the indicated
//               offset vector.
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise2::
grad(int hash, double x, double y) {
  // Convert lo 3 bits of hash code into 8 gradient directions.

  int h = hash & 7;
  return _grad_table[h].dot(LVector2d(x, y));
}
