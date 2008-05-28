// Filename: lorientation_src.I
// Created by:  frang, charles (23Jun00)
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
//     Function: LOrientation::Default Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Copy Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LQuaternion)& c) :
  FLOATNAME(LQuaternion)(c) {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(FLOATTYPE r, FLOATTYPE i, FLOATTYPE j, FLOATTYPE k) :
  FLOATNAME(LQuaternion)(r, i, j, k) {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: vector + twist
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LVector3) &point_at, float twist) {
  float radians = deg_2_rad(twist);
  float theta_over_2 = radians * FLOATCONST(0.5);
  float sin_to2 = sinf(theta_over_2);

  set_r(cosf(theta_over_2));
  set_i(point_at[0] * sin_to2);
  set_j(point_at[1] * sin_to2);
  set_k(point_at[2] * sin_to2);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: matrix3
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LMatrix3) &m) {
  set_from_matrix(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: matrix4
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LMatrix4) &m) {
  set_from_matrix(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::operator *
//       Access: public
//  Description: Orientation * rotation = Orientation
//               Applies a rotation to an orientation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LOrientation) FLOATNAME(LOrientation)::
operator * (const FLOATNAME(LRotation) &other) const {
  return multiply((FLOATNAME(LOrientation) &)other);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::operator *
//       Access: public
//  Description: Orientation * Orientation
//               This is a meaningless operation, and will always
//               simply return the rhs.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LOrientation) FLOATNAME(LOrientation)::
operator * (const FLOATNAME(LQuaternion) &other) const {
  nassert_raise("LOrientation * LQuaternion is undefined; use LOrientation * LRotation or LQuaternion * LQuaternion");
  return multiply((FLOATNAME(LOrientation) &)other);
}
