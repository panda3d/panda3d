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
//     Function: PerlinNoise2::Default Constructor
//       Access: Published
//  Description: The default constructor makes an invalid PerlinNoise2
//               object.  You must at least pass in a scale of each
//               dimension.  
//
//               This constructor exists only so you can create a
//               temporary placeholder PerlinNoise2 object, and later
//               fill it in with the assignment operator.
////////////////////////////////////////////////////////////////////
INLINE PerlinNoise2::
PerlinNoise2() :
  PerlinNoise(0, 1)
{
  _input_xform.fill(0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise2::Copy Constructor
//       Access: Published
//  Description: Makes an exact copy of the existing PerlinNoise
//               object, including its random seed.
////////////////////////////////////////////////////////////////////
INLINE PerlinNoise2::
PerlinNoise2(const PerlinNoise2 &copy) :
  PerlinNoise(copy),
  _input_xform(copy._input_xform)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise2::Copy Assignment Operator
//       Access: Published
//  Description: Makes an exact copy of the existing PerlinNoise
//               object, including its random seed.
////////////////////////////////////////////////////////////////////
INLINE void PerlinNoise2::
operator = (const PerlinNoise2 &copy) {
  PerlinNoise::operator = (copy);
  _input_xform = copy._input_xform;
}

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
//     Function: PerlinNoise2::operator ()
//       Access: Published
//  Description: Returns the noise function of the two inputs.
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise2::
operator ()(double x, double y) {
  return noise(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the two inputs.
////////////////////////////////////////////////////////////////////
INLINE float PerlinNoise2::
operator ()(const LVecBase2f &value) {
  return noise(value);
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the two inputs.
////////////////////////////////////////////////////////////////////
INLINE double PerlinNoise2::
operator ()(const LVecBase2d &value) {
  return noise(value);
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
  // Convert low 3 bits of hash code into 8 gradient directions.
  switch (hash & 7) {
    // Four corners.
  case 0: return x + y;
  case 1: return x - y;
  case 2: return -x + y;
  case 3: return -x - y;

    // Four edges.  Here we scale by 1.707 to make all the vectors equal
    // length, and to make their lengths consistent with PerlinNoise3.
  case 4: return 1.707 * x;
  case 5: return 1.707 * y;
  case 6: return -1.707 * x;
  case 7: return -1.707 * y;
  }

  nassertr(false, 0);
  return 0;
}
