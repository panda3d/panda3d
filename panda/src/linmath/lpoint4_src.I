// Filename: lpoint4_src.I
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
//     Function: LPoint4::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(const FLOATNAME(LVecBase4) &copy) : FLOATNAME(LVecBase4)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
operator = (const FLOATNAME(LVecBase4) &copy) {
  FLOATNAME(LVecBase4)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Fill Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
operator = (FLOATTYPE fill_value) {
  FLOATNAME(LVecBase4)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase4)(fill_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) :
  FLOATNAME(LVecBase4)(x, y, z, w)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(const FLOATNAME(LVecBase3) &copy, FLOATTYPE w) :
  FLOATNAME(LVecBase4)(copy, w)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
zero() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
unit_x() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::unit_x();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
unit_y() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::unit_y();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
unit_z() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::unit_z();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_w Named Constructor
//       Access: Public
//  Description: Returns a unit W point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
unit_w() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::unit_w();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unary -
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator - () const {
  return FLOATNAME(LVecBase4)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point + vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LPoint4)::
operator + (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point + vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator + (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - point
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LPoint4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::operator * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint4)(FLOATNAME(LVecBase4)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::operator / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint4)(FLOATNAME(LVecBase4)::operator / (scalar));
}

#ifndef FLOATTYPE_IS_INT
////////////////////////////////////////////////////////////////////
//     Function: LPoint4::project
//       Access: Published
//  Description: Returns a new vector representing the projection of
//               this vector onto another one.  The resulting vector
//               will be a scalar multiple of onto.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
project(const FLOATNAME(LVecBase4) &onto) const {
  return FLOATNAME(LVecBase4)::project(onto);
}
#endif  // FLOATTYPE_IS_INT
