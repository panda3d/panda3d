// Filename: lvector2_src.I
// Created by:  drose (08Mar00)
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
//     Function: LVector2::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2)::
FLOATNAME(LVector2)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2)::
FLOATNAME(LVector2)(const FLOATNAME(LVecBase2) &copy) : FLOATNAME(LVecBase2)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) &FLOATNAME(LVector2)::
operator = (const FLOATNAME(LVecBase2) &copy) {
  FLOATNAME(LVecBase2)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Fill Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) &FLOATNAME(LVector2)::
operator = (FLOATTYPE fill_value) {
  FLOATNAME(LVecBase2)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2)::
FLOATNAME(LVector2)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase2)(fill_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2)::
FLOATNAME(LVector2)(FLOATTYPE x, FLOATTYPE y) :
  FLOATNAME(LVecBase2)(x, y)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector2) &FLOATNAME(LVector2)::
zero() {
  return (const FLOATNAME(LVector2) &)FLOATNAME(LVecBase2)::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector2) &FLOATNAME(LVector2)::
unit_x() {
  return (const FLOATNAME(LVector2) &)FLOATNAME(LVecBase2)::unit_x();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector2) &FLOATNAME(LVector2)::
unit_y() {
  return (const FLOATNAME(LVector2) &)FLOATNAME(LVecBase2)::unit_y();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unary -
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator - () const {
  return FLOATNAME(LVecBase2)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector + vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LVector2)::
operator + (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector + vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator + (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector - vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LVector2)::
operator - (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector - vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator - (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::operator * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVector2)(FLOATNAME(LVecBase2)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::operator / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LVector2)(FLOATNAME(LVecBase2)::operator / (scalar));
}

#ifndef FLOATTYPE_IS_INT
////////////////////////////////////////////////////////////////////
//     Function: LVector2::project
//       Access: Published
//  Description: Returns a new vector representing the projection of
//               this vector onto another one.  The resulting vector
//               will be a scalar multiple of onto.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
project(const FLOATNAME(LVecBase2) &onto) const {
  return FLOATNAME(LVecBase2)::project(onto);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::signed_angle_rad
//       Access: Published
//  Description: returns the signed angled between two vectors. 
//               normalization is NOT necessary
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector2)::
signed_angle_rad(const FLOATNAME(LVector2) &other) const {
  return catan2((_v(0)*other._v(1))-(_v(1)*other._v(0)), dot(other));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::signed_angle_deg
//       Access: Published
//  Description: returns the signed angled between two vectors. 
//               normalization is NOT necessary
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector2)::
signed_angle_deg(const FLOATNAME(LVector2) &other) const {
  return rad_2_deg(signed_angle_rad(other));
}
#endif  // FLOATTYPE_IS_INT
