// Filename: perlinNoise3.cxx
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

#include "perlinNoise3.h"
#include "cmath.h"

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise3::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
double PerlinNoise3::
noise(const LVecBase3d &value) const {
  // Convert the vector to our local coordinate space.
  LVecBase3d vec = _input_xform.xform_point(value);

  double x = vec[0];
  double y = vec[1];
  double z = vec[2];

  // Find unit cube that contains point.
  double xf = cfloor(x);
  double yf = cfloor(y);
  double zf = cfloor(z);

  int X = ((int)xf) & _table_size_mask;
  int Y = ((int)yf) & _table_size_mask;
  int Z = ((int)zf) & _table_size_mask;

  // Find relative x,y,z of point in cube.
  x -= xf;
  y -= yf;        
  z -= zf;

  // Compute fade curves for each of x,y,z.
  double u = fade(x);
  double v = fade(y);
  double w = fade(z);

  // Hash coordinates of the 8 cube corners.  The 8 corners correspond
  // to AA, BA, AB, BB, AA + 1, BA + 1, AB + 1, and BB + 1.
  int A = _index[X] + Y;
  int AA = _index[A] + Z;
  int AB = _index[A + 1] + Z;
  int B = _index[X + 1] + Y;
  int BA = _index[B] + Z;
  int BB = _index[B + 1] + Z;     
  
  // and add blended results from 8 corners of cube.
  double result =
    lerp(w, lerp(v, lerp(u, grad(_index[AA], x, y, z), 
                         grad(_index[BA], x - 1, y, z)), 
                 lerp(u, grad(_index[AB], x, y - 1, z), 
                      grad(_index[BB], x - 1, y - 1, z))), 
         lerp(v, lerp(u, grad(_index[AA + 1], x, y, z - 1), 
                      grad(_index[BA + 1], x - 1, y, z - 1)), 
              lerp(u, grad(_index[AB + 1], x, y - 1, z - 1), 
                   grad(_index[BB + 1], x - 1, y - 1, z - 1))));

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise3::init_unscaled_xform
//       Access: Private
//  Description: Come up with a random rotation to apply to the input
//               coordinates. This will reduce the problem of the
//               singularities on the axes, by sending the axes in
//               some crazy direction.
////////////////////////////////////////////////////////////////////
void PerlinNoise3::
init_unscaled_xform() {
  LRotationd rot(_randomizer.random_real_unit(),
                 _randomizer.random_real_unit(),
                 _randomizer.random_real_unit(),
                 _randomizer.random_real_unit());
  rot.normalize();
  rot.extract_to_matrix(_unscaled_xform);

  // And come up with a random translation too, just so the
  // singularity at (0, 0, 0) is also unpredicatable.
  _unscaled_xform.set_row(3, LVecBase3d(_randomizer.random_real_unit(),
                                        _randomizer.random_real_unit(),
                                        _randomizer.random_real_unit()));
}
