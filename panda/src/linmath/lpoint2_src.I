// Filename: lpoint2_src.I
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
//     Function: LPoint2::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(const FLOATNAME(LVecBase2) &copy) : FLOATNAME(LVecBase2)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
operator = (const FLOATNAME(LVecBase2) &copy) {
  FLOATNAME(LVecBase2)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Fill Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
operator = (FLOATTYPE fill_value) {
  FLOATNAME(LVecBase2)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase2)(fill_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(FLOATTYPE x, FLOATTYPE y) :
  FLOATNAME(LVecBase2)(x, y)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
zero() {
  return (const FLOATNAME(LPoint2) &)FLOATNAME(LVecBase2)::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
unit_x() {
  return (const FLOATNAME(LPoint2) &)FLOATNAME(LVecBase2)::unit_x();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
unit_y() {
  return (const FLOATNAME(LPoint2) &)FLOATNAME(LVecBase2)::unit_y();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unary -
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator - () const {
  return FLOATNAME(LVecBase2)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point + vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LPoint2)::
operator + (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point + vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator + (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - point
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LPoint2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::operator * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint2)(FLOATNAME(LVecBase2)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::operator / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint2)(FLOATNAME(LVecBase2)::operator / (scalar));
}

#ifndef FLOATTYPE_IS_INT
////////////////////////////////////////////////////////////////////
//     Function: LPoint2::project
//       Access: Published
//  Description: Returns a new vector representing the projection of
//               this vector onto another one.  The resulting vector
//               will be a scalar multiple of onto.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
project(const FLOATNAME(LVecBase2) &onto) const {
  return FLOATNAME(LVecBase2)::project(onto);
}
#endif  // FLOATTYPE_IS_INT
