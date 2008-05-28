// Filename: rotate_to_src.cxx
// Created by:  drose (04Nov99)
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
//     Function: _rotate_to
//  Description: Computes the matrix necessary to rotate vector a onto
//               vector b.  It is assumed that both vectors are
//               normalized.
////////////////////////////////////////////////////////////////////
static void
_rotate_to(FLOATNAME(LMatrix3) &mat,
           const FLOATNAME(LVector3) &a, const FLOATNAME(LVector3) &b) {
  FLOATTYPE cos_theta = a.dot(b);

  FLOATNAME(LVector3) axis = a.cross(b);
  FLOATTYPE sin_theta = length(axis);

  // Check for collinear vectors
  if (sin_theta < 0.0001) {
    // The vectors are collinear.

    if (cos_theta < 0.0f) {
      // The vectors are opposite; choose an arbitrary axis
      // perpendicular to a.
      FLOATNAME(LVector3) absa(fabs(a[0]), fabs(a[1]), fabs(a[2]));
      FLOATNAME(LVector3) lca(0., 0., 0.);
      lca[absa[0]<=absa[1] ? absa[0]<=absa[2] ? 0 : 2
         : absa[1]<=absa[2] ? 1 : 2] = 1.0f;

      axis = normalize(a.cross(lca));
    } else {
      mat = FLOATNAME(LMatrix3)::ident_mat();
      return;
    }

  } else {
    // The vectors are not collinear; determine the best axis.
    axis /= sin_theta;
  }

  FLOATTYPE x = axis[0];
  FLOATTYPE y = axis[1];
  FLOATTYPE z = axis[2];

  FLOATTYPE t = 1.0f - cos_theta;

  mat(0, 0) = t * x * x + cos_theta;
  mat(0, 1) = t * x * y + sin_theta * z;
  mat(0, 2) = t * x * z - sin_theta * y;

  mat(1, 0) = t * y * x - sin_theta * z;
  mat(1, 1) = t * y * y + cos_theta;
  mat(1, 2) = t * y * z + sin_theta * x;

  mat(2, 0) = t * z * x + sin_theta * y;
  mat(2, 1) = t * z * y - sin_theta * x;
  mat(2, 2) = t * z * z + cos_theta;
}


void
rotate_to(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &a, const FLOATNAME(LVector3) &b) {
  _rotate_to(mat, a, b);
}

void
rotate_to(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &a, const FLOATNAME(LVector3) &b) {
  FLOATNAME(LMatrix3) m3;
  _rotate_to(m3, a, b);
  mat = FLOATNAME(LMatrix4)(m3);
}

