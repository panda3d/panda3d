// Filename: linearRandomForce.I
// Created by:  charles (19Jun00)
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
//     Function : random_unit_vector
//       Access : Protected
//  Description : generates a random unit vector
////////////////////////////////////////////////////////////////////
INLINE LVector3f LinearRandomForce::
random_unit_vector(void) {
  float z, r, theta;

  z = 1.0f - (2.0f * bounded_rand());
  r = sqrtf(1.0f - (z * z));
  theta = 2.0f * 3.1415926f * bounded_rand();

  return LVector3f(r * cosf(theta), r * sinf(theta), z);
}

