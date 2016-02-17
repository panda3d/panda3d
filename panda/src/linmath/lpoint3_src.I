/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lpoint3_src.I
 * @author drose
 * @date 1999-09-25
 */

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(const FLOATNAME(LVecBase3) &copy) : FLOATNAME(LVecBase3)(copy) {
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase3)(fill_value)
{
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) :
  FLOATNAME(LVecBase3)(x, y, z)
{
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(const FLOATNAME(LVecBase2) &copy, FLOATTYPE z) :
  FLOATNAME(LVecBase3)(copy, z)
{
}

/**
 * Returns a zero-length point.
 */
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
zero() {
  return (const FLOATNAME(LPoint3) &)FLOATNAME(LVecBase3)::zero();
}

/**
 * Returns a unit X point.
 */
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
unit_x() {
  return (const FLOATNAME(LPoint3) &)FLOATNAME(LVecBase3)::unit_x();
}

/**
 * Returns a unit Y point.
 */
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
unit_y() {
  return (const FLOATNAME(LPoint3) &)FLOATNAME(LVecBase3)::unit_y();
}

/**
 * Returns a unit Z point.
 */
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
unit_z() {
  return (const FLOATNAME(LPoint3) &)FLOATNAME(LVecBase3)::unit_z();
}

/**
 * Returns a 2-component vector that shares just the first two components of
 * this vector.
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint3)::
get_xy() const {
  return FLOATNAME(LPoint2)(_v(0), _v(1));
}

/**
 * Returns a 2-component vector that shares just the first and last components
 * of this vector.
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint3)::
get_xz() const {
  return FLOATNAME(LPoint2)(_v(0), _v(2));
}

/**
 * Returns a 2-component vector that shares just the last two components of
 * this vector.
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint3)::
get_yz() const {
  return FLOATNAME(LPoint2)(_v(1), _v(2));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator - () const {
  return FLOATNAME(LVecBase3)::operator - ();
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LPoint3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator + (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LPoint3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::cross(other);
}

#ifndef FLOATTYPE_IS_INT
/**
 * Normalizes the vector and returns the normalized vector as a copy.  If the
 * vector was a zero-length vector, a zero length vector will be returned.
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
normalized() const {
  return FLOATNAME(LVecBase3)::normalized();
}

/**
 * Returns a new vector representing the projection of this vector onto
 * another one.  The resulting vector will be a scalar multiple of onto.
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
project(const FLOATNAME(LVecBase3) &onto) const {
  return FLOATNAME(LVecBase3)::project(onto);
}
#endif  // FLOATTYPE_IS_INT

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint3)(FLOATNAME(LVecBase3)::operator * (scalar));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint3)(FLOATNAME(LVecBase3)::operator / (scalar));
}

/**
 * Returns the origin of the indicated coordinate system.  This is always 0,
 * 0, 0 with all of our existing coordinate systems; it's hard to imagine it
 * ever being different.
 */
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
origin(CoordinateSystem) {
  return FLOATNAME(LPoint3)::zero();
}

/**
 * Returns a point described by right, forward, up displacements from the
 * origin, wherever that maps to in the given coordinate system.
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
rfu(FLOATTYPE right_v, FLOATTYPE fwd_v, FLOATTYPE up_v,
    CoordinateSystem cs) {
  return origin(cs) +
    FLOATNAME(LVector3)::rfu(right_v, fwd_v, up_v, cs);
}
