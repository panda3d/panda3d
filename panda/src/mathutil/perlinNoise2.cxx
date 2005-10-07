// Filename: perlinNoise2.cxx
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

#include "perlinNoise2.h"
#include "cmath.h"

LVector2d PerlinNoise2::_grad_table[8] = {
  // Four corners.
  LVector2d(1, 1),
  LVector2d(1, -1),
  LVector2d(-1, 1),
  LVector2d(-1, -1),

  // Four edges.  Here we scale by 1.707 to make all the vectors equal
  // length, and to make their lengths consistent with PerlinNoise3.
  LVector2d(1.707, 0),
  LVector2d(0, 1.707),
  LVector2d(-1.707, 0),
  LVector2d(0, -1.707),
};

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise2::Constructor
//       Access: Published
//  Description: Randomizes the tables to make a unique noise
//               function.
//
//               If seed is nonzero, it is used to define the tables;
//               if it is zero a random seed is generated.
////////////////////////////////////////////////////////////////////
PerlinNoise2::
PerlinNoise2(double sx, double sy,
	     int table_size, unsigned long seed) :
  PerlinNoise(table_size, seed)
{
  // Come up with a random rotation to apply to the input coordinates.
  // This will reduce the problem of the singularities on the axes, by
  // sending the axes in some crazy direction.
  double rot = random_real(360.0f);
  _input_xform = LMatrix3d::rotate_mat(rot);

  // And come up with a random translation too, just so the
  // singularity at (0, 0) is also unpredicatable.
  _input_xform.set_row(2, LVecBase2d(random_real_unit(),
				     random_real_unit()));

  // Finally, apply the user's input scale.
  _input_xform = LMatrix3d::scale_mat(1.0f / sx, 1.0f / sy) * _input_xform;
}

////////////////////////////////////////////////////////////////////
//     Function: PerlinNoise2::noise
//       Access: Published
//  Description: Returns the noise function of the three inputs.
////////////////////////////////////////////////////////////////////
double PerlinNoise2::
noise(const LVecBase2d &value) {
  // Convert the vector to our local coordinate space.
  LVecBase2d vec = _input_xform.xform_point(value);

  double x = vec._v.v._0;
  double y = vec._v.v._1;

  // Find unit square that contains point.
  int X = cmod((int)cfloor(x), _table_size);
  int Y = cmod((int)cfloor(y), _table_size);

  // Find relative x,y of point in square.
  x -= cfloor(x);
  y -= cfloor(y);        

  // Compute fade curves for each of x,y.
  double u = fade(x);
  double v = fade(y);

  // Hash coordinates of the 4 square corners . . .
  int A = _index[X] + Y;
  int B = _index[X + 1] + Y;
  
  // . . . and add blended results from 8 corners of cube.
  double result =
    lerp(v, lerp(u, grad(_index[A], x, y), 
                 grad(_index[B], x - 1, y)), 
         lerp(u, grad(_index[A + 1], x, y - 1), 
              grad(_index[B + 1], x - 1, y - 1)));

  return result;
}
