// Filename: lvector2_src.I
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
//     Function: LVector2::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector2)::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector2)::
length_squared() const {
  return (*this).dot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::normalize
//       Access: Public
//  Description: Normalizes the vector in place.  Returns true if the
//               vector was normalized, false if it was a zero-length
//               vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVector2)::
normalize() {
  FLOATTYPE l2 = length_squared();
  if (l2 == (FLOATTYPE)0.0f) {
    set(0.0f, 0.0f);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0f, NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE))) {
    (*this) /= csqrt(l2);
  }

  return true;
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
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return FLOATNAME(LVector2)(FLOATNAME(LVecBase2)::operator * (recip_scalar));
}
