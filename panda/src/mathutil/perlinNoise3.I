// Filename: perlinNoise3.I
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
//     Function: PerlinNoise3::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise3::
noise(double x, double y, double z) {
  return noise(LVecBase3d(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise3::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
INLINE float PerlinNoise3::
noise(const LVecBase3f &value) {
  return (float)noise(value[0], value[1], value[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise3::grad
//       Access: Private, Static
//  Description: Returns the dot product of a random gradient vector
//               (determined by the hash code) with the indicated
//               offset vector.
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise3::
grad(int hash, double x, double y, double z) {
  // Convert lo 4 bits of hash code into 12 gradient directions.

  int h = hash & 15;
  return _grad_table[h].dot(LVector3d(x, y, z));

  /*
    This is Perlin's reference code, but the table lookup above is
    slightly faster (no jump instructions) and produces exactly the
    same results.
  double u = (h < 8) ? x : y;
  double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
  return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
  */
}
