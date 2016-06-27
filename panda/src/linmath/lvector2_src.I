/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvector2_src.I
 * @author drose
 * @date 2000-03-08
 */

/**
 * Constructs a new LVector2 from a LVecBase2
 */
INLINE_LINMATH FLOATNAME(LVector2)::
FLOATNAME(LVector2)(const FLOATNAME(LVecBase2)& copy) :
  FLOATNAME(LVecBase2)(copy)
{
}

/**
 * Constructs a new LVector2 with all components set to the fill value.
 */
INLINE_LINMATH FLOATNAME(LVector2)::
FLOATNAME(LVector2)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase2)(fill_value)
{
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector2)::
FLOATNAME(LVector2)(FLOATTYPE x, FLOATTYPE y) :
  FLOATNAME(LVecBase2)(x, y)
{
}

/**
 * Returns a zero-length vector.
 */
INLINE_LINMATH const FLOATNAME(LVector2) &FLOATNAME(LVector2)::
zero() {
  return (const FLOATNAME(LVector2) &)FLOATNAME(LVecBase2)::zero();
}

/**
 * Returns a unit X vector.
 */
INLINE_LINMATH const FLOATNAME(LVector2) &FLOATNAME(LVector2)::
unit_x() {
  return (const FLOATNAME(LVector2) &)FLOATNAME(LVecBase2)::unit_x();
}

/**
 * Returns a unit Y vector.
 */
INLINE_LINMATH const FLOATNAME(LVector2) &FLOATNAME(LVector2)::
unit_y() {
  return (const FLOATNAME(LVector2) &)FLOATNAME(LVecBase2)::unit_y();
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator - () const {
  return FLOATNAME(LVecBase2)::operator - ();
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LVector2)::
operator + (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator + (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LVector2)::
operator - (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator - (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVector2)(FLOATNAME(LVecBase2)::operator * (scalar));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LVector2)(FLOATNAME(LVecBase2)::operator / (scalar));
}

#ifndef FLOATTYPE_IS_INT
/**
 * Normalizes the vector and returns the normalized vector as a copy.  If the
 * vector was a zero-length vector, a zero length vector will be returned.
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
normalized() const {
  return FLOATNAME(LVecBase2)::normalized();
}

/**
 * Returns a new vector representing the projection of this vector onto
 * another one.  The resulting vector will be a scalar multiple of onto.
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LVector2)::
project(const FLOATNAME(LVecBase2) &onto) const {
  return FLOATNAME(LVecBase2)::project(onto);
}

/**
 * returns the signed angled between two vectors.  normalization is NOT
 * necessary
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector2)::
signed_angle_rad(const FLOATNAME(LVector2) &other) const {
  return catan2((_v(0)*other._v(1))-(_v(1)*other._v(0)), dot(other));
}

/**
 * returns the signed angled between two vectors.  normalization is NOT
 * necessary
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVector2)::
signed_angle_deg(const FLOATNAME(LVector2) &other) const {
  return rad_2_deg(signed_angle_rad(other));
}
#endif  // FLOATTYPE_IS_INT
