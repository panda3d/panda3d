// Filename: lrotation_src.I
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
//     Function: LRotation::Default Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation)::
FLOATNAME(LRotation)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Copy Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LQuaternion) &c) :
  FLOATNAME(LQuaternion)(c) {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Copy Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LVecBase4) &copy) :
  FLOATNAME(LQuaternion)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation)::
FLOATNAME(LRotation)(FLOATTYPE r, FLOATTYPE i, FLOATTYPE j, FLOATTYPE k) :
  FLOATNAME(LQuaternion)(r, i, j, k) {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: lmatrix3
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LMatrix3) &m) {
  set_from_matrix(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: lmatrix4
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LMatrix4) &m) {
  set_from_matrix(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: axis + angle (in degrees)
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LVector3) &axis, FLOATTYPE angle) {
  FLOATTYPE radians = deg_2_rad(angle);
  FLOATTYPE theta_over_2 = radians * FLOATCONST(0.5);
  FLOATTYPE sin_to2 = csin(theta_over_2);

  set_r(ccos(theta_over_2));
  set_i(axis[0] * sin_to2);
  set_j(axis[1] * sin_to2);
  set_k(axis[2] * sin_to2);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: Sets the rotation from the given Euler angles.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation)::
FLOATNAME(LRotation)(FLOATTYPE h, FLOATTYPE p, FLOATTYPE r) {
  set_hpr(FLOATNAME(LVecBase3)(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::rotation * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation) FLOATNAME(LRotation)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LRotation)(FLOATNAME(LVecBase4)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::rotation / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation) FLOATNAME(LRotation)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LRotation)(FLOATNAME(LVecBase4)::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::operator *
//       Access: public
//  Description: Rotation * Rotation = Rotation
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LRotation) FLOATNAME(LRotation)::
operator * (const FLOATNAME(LRotation) &other) const {
  return multiply(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::operator *
//       Access: public
//  Description: Rotation * Orientation = Orientation
//               This is another meaningless operation, attempting
//               to apply an orientation to a rotation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LRotation)::
operator * (const FLOATNAME(LQuaternion) &other) const {
  nassert_raise("LRotation * LQuaternion is undefined; use LRotation * LRotation or LQuaternion * LQuaternion");
  return multiply(other);
}


