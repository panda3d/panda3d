// Filename: lvector4_src.I
// Created by:  drose (08Mar00)
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
//     Function: LVector4::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector4)::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector4)::
length_squared() const {
  return (*this).dot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::normalize
//       Access: Public
//  Description: Normalizes the vector in place.  Returns true if the
//               vector was normalized, false if it was a zero-length
//               vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVector4)::
normalize() {
  FLOATTYPE l2 = length_squared();
  if (l2 == (FLOATTYPE)0.0f) {
    set(0.0f, 0.0f, 0.0f, 0.0f);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0f, NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE))) {
    (*this) /= csqrt(l2);
  }

  return true;
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
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return FLOATNAME(LVector4)(FLOATNAME(LVecBase4)::operator * (recip_scalar));
}
