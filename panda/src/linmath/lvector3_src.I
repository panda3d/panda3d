/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvector3_src.I
 * @author drose
 * @date 1999-09-24
 */

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3)::
FLOATNAME(LVector3)(const FLOATNAME(LVecBase3) &copy) : FLOATNAME(LVecBase3)(copy) {
}


/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3)::
FLOATNAME(LVector3)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase3)(fill_value)
{
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3)::
FLOATNAME(LVector3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) :
  FLOATNAME(LVecBase3)(x, y, z)
{
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3)::
FLOATNAME(LVector3)(const FLOATNAME(LVecBase2) &copy, FLOATTYPE z) :
  FLOATNAME(LVecBase3)(copy, z)
{
}

/**
 * Returns a zero-length vector.
 */
INLINE_LINMATH const FLOATNAME(LVector3) &FLOATNAME(LVector3)::
zero() {
  return (const FLOATNAME(LVector3) &)FLOATNAME(LVecBase3)::zero();
}

/**
 * Returns a unit X vector.
 */
INLINE_LINMATH const FLOATNAME(LVector3) &FLOATNAME(LVector3)::
unit_x() {
  return (const FLOATNAME(LVector3) &)FLOATNAME(LVecBase3)::unit_x();
}

/**
 * Returns a unit Y vector.
 */
INLINE_LINMATH const FLOATNAME(LVector3) &FLOATNAME(LVector3)::
unit_y() {
  return (const FLOATNAME(LVector3) &)FLOATNAME(LVecBase3)::unit_y();
}

/**
 * Returns a unit Z vector.
 */
INLINE_LINMATH const FLOATNAME(LVector3) &FLOATNAME(LVector3)::
unit_z() {
  return (const FLOATNAME(LVector3) &)FLOATNAME(LVecBase3)::unit_z();
}

/**
 * Returns a 2-component vector that shares just the first two components of
 * this vector.
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector3)::
get_xy() const {
  return FLOATNAME(LVector2)(_v(0), _v(1));
}

/**
 * Returns a 2-component vector that shares just the first and last components
 * of this vector.
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector3)::
get_xz() const {
  return FLOATNAME(LVector2)(_v(0), _v(2));
}

/**
 * Returns a 2-component vector that shares just the last two components of
 * this vector.
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector3)::
get_yz() const {
  return FLOATNAME(LVector2)(_v(1), _v(2));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator - () const {
  return FLOATNAME(LVecBase3)::operator - ();
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVector3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator + (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVector3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator - (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::cross(other);
}

#ifndef FLOATTYPE_IS_INT

/**
 * Normalizes the vector and returns the normalized vector as a copy.  If the
 * vector was a zero-length vector, a zero length vector will be returned.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
normalized() const {
  return FLOATNAME(LVecBase3)::normalized();
}

/**
 * Returns a new vector representing the projection of this vector onto
 * another one.  The resulting vector will be a scalar multiple of onto.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
project(const FLOATNAME(LVecBase3) &onto) const {
  return FLOATNAME(LVecBase3)::project(onto);
}

/**
 * Returns the unsigned angle between this vector and the other one, expressed
 * in radians.  Both vectors should be initially normalized.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
angle_rad(const FLOATNAME(LVector3) &other) const {
  // This algorithm yields better results than acos(dot(other)), which behaves
  // poorly as dot(other) approaches 1.0.
  if (dot(other) < 0.0f) {
    FLOATTYPE a = ((*this)+other).length() / 2.0f;
    return MathNumbers::cpi((FLOATTYPE)0.0f) - 2.0f * casin(std::min(a, (FLOATTYPE)1.0));
  } else {
    FLOATTYPE a = ((*this)-other).length() / 2.0f;
    return 2.0f * casin(std::min(a, (FLOATTYPE)1.0));
  }
}

/**
 * Returns the angle between this vector and the other one, expressed in
 * degrees.  Both vectors should be initially normalized.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
angle_deg(const FLOATNAME(LVector3) &other) const {
  return rad_2_deg(angle_rad(other));
}

/**
 * returns the signed angle between two vectors.  The angle is positive if the
 * rotation from this vector to other is clockwise when looking in the
 * direction of the ref vector.
 *
 * Vectors (except the ref vector) should be initially normalized.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
signed_angle_rad(const FLOATNAME(LVector3) &other,
                 const FLOATNAME(LVector3) &ref) const {
  FLOATTYPE angle = angle_rad(other);
  if (cross(other).dot(ref) < 0.0f) {
    angle = -angle;
  }
  return angle;
}

/**
 * Returns the signed angle between two vectors.  The angle is positive if the
 * rotation from this vector to other is clockwise when looking in the
 * direction of the ref vector.
 *
 * Vectors (except the ref vector) should be initially normalized.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
signed_angle_deg(const FLOATNAME(LVector3) &other,
                 const FLOATNAME(LVector3) &ref) const {
  return rad_2_deg(signed_angle_rad(other, ref));
}

/**
 * @deprecated Do not use.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
relative_angle_rad(const FLOATNAME(LVector3) &other) const {
  return atan2((_v(0)*other._v(1))-(_v(1)*other._v(0)), dot(other));
}

/**
 * @deprecated Do not use.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector3)::
relative_angle_deg(const FLOATNAME(LVector3) &other) const {
  return relative_angle_rad(other) * FLOATCONST(180.0) / FLOATCONST(3.1415926535);
}
#endif  // FLOATTYPE_IS_INT

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVector3)(FLOATNAME(LVecBase3)::operator * (scalar));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LVector3)(FLOATNAME(LVecBase3)::operator / (scalar));
}

/**
 * Returns the up vector for the given coordinate system.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
up(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  switch (cs) {
  case CS_zup_right:
  case CS_zup_left:
    return FLOATNAME(LVector3)(0, 0, 1);

  case CS_yup_right:
  case CS_yup_left:
    return FLOATNAME(LVector3)(0, 1, 0);

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LVector3)(0, 0, 0);
  }
}

/**
 * Returns the right vector for the given coordinate system.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
right(CoordinateSystem) {
  return FLOATNAME(LVector3)(1, 0, 0);
}

/**
 * Returns the forward vector for the given coordinate system.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
forward(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  switch (cs) {
  case CS_zup_right:
    return FLOATNAME(LVector3)(0, 1, 0);

  case CS_zup_left:
    return FLOATNAME(LVector3)(0, -1, 0);

  case CS_yup_right:
    return FLOATNAME(LVector3)(0, 0, -1);

  case CS_yup_left:
    return FLOATNAME(LVector3)(0, 0, 1);

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LVector3)(0, 0, 0);
  }
}

/**
 * Returns the down vector for the given coordinate system.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
down(CoordinateSystem cs) {
  return -up(cs);
}

/**
 * Returns the left vector for the given coordinate system.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
left(CoordinateSystem cs) {
  return -right(cs);
}

/**
 * Returns the back vector for the given coordinate system.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
back(CoordinateSystem cs) {
  return -forward(cs);
}

/**
 * Returns a vector that is described by its right, forward, and up
 * components, in whatever way the coordinate system represents that vector.
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LVector3)::
rfu(FLOATTYPE right_v, FLOATTYPE fwd_v, FLOATTYPE up_v,
    CoordinateSystem cs) {

  /*  return forward(cs) * fwd_v + up(cs) * up_v + right(cs) * right_v; */

  // fast implementation of above for axis-aligned coordinate systems
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  FLOATTYPE vy, vz;
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
    break;

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LVector3)(0);
  }

  return FLOATNAME(LVector3)(right_v, vy, vz);
}
