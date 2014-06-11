// Filename: lvector4_src.I
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
//     Function: LVector4::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4)::
FLOATNAME(LVector4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4)::
FLOATNAME(LVector4)(const FLOATNAME(LVecBase4) &copy) : FLOATNAME(LVecBase4)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) &FLOATNAME(LVector4)::
operator = (const FLOATNAME(LVecBase4) &copy) {
  FLOATNAME(LVecBase4)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Fill Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) &FLOATNAME(LVector4)::
operator = (FLOATTYPE fill_value) {
  FLOATNAME(LVecBase4)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4)::
FLOATNAME(LVector4)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase4)(fill_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4)::
FLOATNAME(LVector4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) :
  FLOATNAME(LVecBase4)(x, y, z, w)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4)::
FLOATNAME(LVector4)(const FLOATNAME(LVecBase3) &copy, FLOATTYPE w) :
  FLOATNAME(LVecBase4)(copy, w)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector4) &FLOATNAME(LVector4)::
zero() {
  return (const FLOATNAME(LVector4) &)FLOATNAME(LVecBase4)::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector4) &FLOATNAME(LVector4)::
unit_x() {
  return (const FLOATNAME(LVector4) &)FLOATNAME(LVecBase4)::unit_x();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector4) &FLOATNAME(LVector4)::
unit_y() {
  return (const FLOATNAME(LVector4) &)FLOATNAME(LVecBase4)::unit_y();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector4) &FLOATNAME(LVector4)::
unit_z() {
  return (const FLOATNAME(LVector4) &)FLOATNAME(LVecBase4)::unit_z();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_w Named Constructor
//       Access: Public
//  Description: Returns a unit W vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector4) &FLOATNAME(LVector4)::
unit_w() {
  return (const FLOATNAME(LVector4) &)FLOATNAME(LVecBase4)::unit_w();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unary -
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator - () const {
  return FLOATNAME(LVecBase4)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector + vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVector4)::
operator + (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector + vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator + (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector - vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVector4)::
operator - (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector - vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator - (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::operator * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVector4)(FLOATNAME(LVecBase4)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::operator / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LVector4)(FLOATNAME(LVecBase4)::operator / (scalar));
}

#ifndef FLOATTYPE_IS_INT
////////////////////////////////////////////////////////////////////
//     Function: LVector4::project
//       Access: Published
//  Description: Returns a new vector representing the projection of
//               this vector onto another one.  The resulting vector
//               will be a scalar multiple of onto.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4) FLOATNAME(LVector4)::
project(const FLOATNAME(LVecBase4) &onto) const {
  return FLOATNAME(LVecBase4)::project(onto);
}
#endif  // FLOATTYPE_IS_INT
