// Filename: lvector3_src.I
// Created by:  drose (24Sep99)
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
//     Function: LVector3::Default Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3)::
FLOATNAME(LVector3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3)::
FLOATNAME(LVector3)(const FLOATNAME(LVecBase3) &copy) : FLOATNAME(LVecBase3)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) &FLOATNAME(LVector3)::
operator = (const FLOATNAME(LVecBase3) &copy) {
  FLOATNAME(LVecBase3)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Fill Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) &FLOATNAME(LVector3)::
operator = (FLOATTYPE fill_value) {
  FLOATNAME(LVecBase3)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3)::
FLOATNAME(LVector3)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase3)(fill_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3)::
FLOATNAME(LVector3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) :
  FLOATNAME(LVecBase3)(x, y, z)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::zero Named Constructor
//       Access: Published
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector3) &FLOATNAME(LVector3)::
zero() {
  return (const FLOATNAME(LVector3) &)FLOATNAME(LVecBase3)::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_x Named Constructor
//       Access: Published
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector3) &FLOATNAME(LVector3)::
unit_x() {
  return (const FLOATNAME(LVector3) &)FLOATNAME(LVecBase3)::unit_x();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_y Named Constructor
//       Access: Published
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector3) &FLOATNAME(LVector3)::
unit_y() {
  return (const FLOATNAME(LVector3) &)FLOATNAME(LVecBase3)::unit_y();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_z Named Constructor
//       Access: Published
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVector3) &FLOATNAME(LVector3)::
unit_z() {
  return (const FLOATNAME(LVector3) &)FLOATNAME(LVecBase3)::unit_z();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unary -
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator - () const {
  return FLOATNAME(LVecBase3)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector + vecbase
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVector3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector + vector
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator + (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector - vecbase
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVector3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector - vector
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator - (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::length
//       Access: Published
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::length_squared
//       Access: Published
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
length_squared() const {
  return (*this).dot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::normalize
//       Access: Published
//  Description: Normalizes the vector in place.  Returns true if the
//               vector was normalized, false if it was a zero-length
//               vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVector3)::
normalize() {
  FLOATTYPE l2 = length_squared();
  if (l2 == (FLOATTYPE)0.0f) {
    set(0.0f, 0.0f, 0.0f);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0f, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::cross
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::cross(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector::angle_rad
//       Access: Published
//  Description: Returns the angle between this vector and the other
//               one, expressed in radians.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
angle_rad(const FLOATNAME(LVector3) &other) const {
  // This algorithm yields better results than acos(dot(other)), which
  // behaves poorly as dot(other) approaches 1.0.
  if (dot(other) < 0.0f) {
    return MathNumbers::cpi((FLOATTYPE)0.0f) - 2.0f * casin((-(*this)-other).length());
  } else {
    return 2.0f * casin(((*this)-other).length() / 2.0f);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LVector::angle_deg
//       Access: Published
//  Description: Returns the angle between this vector and the other
//               one, expressed in degrees.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
angle_deg(const FLOATNAME(LVector3) &other) const {
  return rad_2_deg(angle_rad(other));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::operator * scalar
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVector3)(FLOATNAME(LVecBase3)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::operator / scalar
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator / (FLOATTYPE scalar) const {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return FLOATNAME(LVector3)(FLOATNAME(LVecBase3)::operator * (recip_scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::up
//       Access: Published, Static
//  Description: Returns the up vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
up(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  switch (cs) {
  case CS_zup_right:
  case CS_zup_left:
    return FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f);

  case CS_yup_right:
  case CS_yup_left:
    return FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f);

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LVector3)(0.0f, 0.0f, 0.0f);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::right
//       Access: Published, Static
//  Description: Returns the right vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
right(CoordinateSystem) {
  return FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::forward
//       Access: Published, Static
//  Description: Returns the forward vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
forward(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  switch (cs) {
  case CS_zup_right:
    return FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f);

  case CS_zup_left:
    return FLOATNAME(LVector3)(0.0f, -1.0f, 0.0f);

  case CS_yup_right:
    return FLOATNAME(LVector3)(0.0f, 0.0f, -1.0f);

  case CS_yup_left:
    return FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f);

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LVector3)(0.0f, 0.0f, 0.0f);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::down
//       Access: Published, Static
//  Description: Returns the down vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
down(CoordinateSystem cs) {
  return -up(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::left
//       Access: Published, Static
//  Description: Returns the left vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
left(CoordinateSystem cs) {
  return -right(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::back
//       Access: Published, Static
//  Description: Returns the back vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
back(CoordinateSystem cs) {
  return -forward(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::rfu
//       Access: Published, Static
//  Description: Returns a vector that is described by its right,
//               forward, and up components, in whatever way the
//               coordinate system represents that vector.
////////////////////////////////////////////////////////////////////

//INLINE_LINMATH FLOATNAME(LVector3) & FLOATNAME(LVector3)::
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
rfu(FLOATTYPE right_v, FLOATTYPE fwd_v, FLOATTYPE up_v,
    CoordinateSystem cs) {

  /*  return forward(cs) * fwd_v + up(cs) * up_v + right(cs) * right_v; */

  // fast implementation of above for axis-aligned coordinate systems


  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  FLOATTYPE vy,vz;
  switch(cs) {
  case CS_yup_right:
    vz = -fwd_v;
    vy = up_v;
    break;

  case CS_yup_left:
    vz = fwd_v;
    vy = up_v;
    break;

  case CS_zup_right:
    vy = fwd_v;
    vz = up_v;
    break;

  case CS_zup_left:
    vy = -fwd_v;
    vz = up_v;

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LVector3)(0.0f, 0.0f, 0.0f);
  }

 return FLOATNAME(LVector3)(right_v,vy,vz);
}

