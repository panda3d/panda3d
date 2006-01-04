// Filename: perlinNoise.I
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
//     Function: PerlinNoise::fade
//       Access: Protected, Static
//  Description: Returns a smooth interpolation spline from 0 .. 1
//               for t.
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise::
fade(double t) { 
  //  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
  return (3.0 - 2.0 * t) * t * t;
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::lerp
//       Access: Protected, Static
//  Description: Returns the smoothly lerped value from a to b.
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise::
lerp(double t, double a, double b) { 
  return a + t * (b - a); 
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::random_int
//       Access: Protected
//  Description: Returns a random integer in the range [0, range).
////////////////////////////////////////////////////////////////////
INLINE int PerlinNoise::
random_int(int range) {
  return (int)floor(random_real((double)range));
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::random_real
//       Access: Protected
//  Description: Returns a random double in the range [0, range).
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise::
random_real(double range) {
  return (range * _mersenne.get_uint31()) / ((double)0x80000000);
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::random_real_unit
//       Access: Protected
//  Description: Returns a random double in the range [-0.5, 0.5).
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise::
random_real_unit() {
  return random_real(1.0f) - 0.5f;
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::get_next_seed
//       Access: Protected, Static
//  Description: Returns a random seed value for the next global
//               PerlinNoise object.
////////////////////////////////////////////////////////////////////
INLINE unsigned long PerlinNoise::
get_next_seed() {
  if (!_got_first_seed) {
    _next_seed = Mersenne((unsigned long)time(NULL));
    _got_first_seed = true;
  }
  return _next_seed.get_uint31();
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise::get_seed
//       Access: Published
//  Description: Returns a unique seed value based on the seed value
//               passed to this PerlinNoise object (and on its current
//               state).
////////////////////////////////////////////////////////////////////
INLINE unsigned long PerlinNoise::
get_seed() {
  return _mersenne.get_uint31();
}
