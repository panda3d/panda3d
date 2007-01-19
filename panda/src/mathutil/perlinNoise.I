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
//     Function: PerlinNoise::get_seed
//       Access: Published
//  Description: Returns a unique seed value based on the seed value
//               passed to this PerlinNoise object (and on its current
//               state).
////////////////////////////////////////////////////////////////////
INLINE unsigned long PerlinNoise::
get_seed() {
  return _randomizer.get_seed();
}
