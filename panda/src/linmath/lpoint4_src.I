/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lpoint4_src.I
 * @author drose
 * @date 2000-03-08
 */

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(const FLOATNAME(LVecBase4) &copy) : FLOATNAME(LVecBase4)(copy) {
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase4)(fill_value)
{
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) :
  FLOATNAME(LVecBase4)(x, y, z, w)
{
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(const FLOATNAME(LVecBase3) &copy, FLOATTYPE w) :
  FLOATNAME(LVecBase4)(copy, w)
{
}

/**
 * Returns a zero-length point.
 */
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
zero() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::zero();
}

/**
 * Returns a unit X point.
 */
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
unit_x() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::unit_x();
}

/**
 * Returns a unit Y point.
 */
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
unit_y() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::unit_y();
}

/**
 * Returns a unit Z point.
 */
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
unit_z() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::unit_z();
}

/**
 * Returns a unit W point.
 */
INLINE_LINMATH const FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
unit_w() {
  return (const FLOATNAME(LPoint4) &)FLOATNAME(LVecBase4)::unit_w();
}


/**
 * Returns the x, y and z component of this vector
 */
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint4)::
get_xyz() const {
  return FLOATNAME(LPoint3)(_v(0), _v(1), _v(2));
}

/**
 * Returns the x and y component of this vector
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint4)::
get_xy() const {
  return FLOATNAME(LPoint2)(_v(0), _v(1));
}


/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator - () const {
  return FLOATNAME(LVecBase4)::operator - ();
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LPoint4)::
operator + (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator + (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LPoint4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint4)(FLOATNAME(LVecBase4)::operator * (scalar));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint4)(FLOATNAME(LVecBase4)::operator / (scalar));
}

#ifndef FLOATTYPE_IS_INT

/**
 * Normalizes the vector and returns the normalized vector as a copy.  If the
 * vector was a zero-length vector, a zero length vector will be returned.
 */
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
normalized() const {
  return FLOATNAME(LVecBase4)::normalized();
}

/**
 * Returns a new vector representing the projection of this vector onto
 * another one.  The resulting vector will be a scalar multiple of onto.
 */
INLINE_LINMATH FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
project(const FLOATNAME(LVecBase4) &onto) const {
  return FLOATNAME(LVecBase4)::project(onto);
}
#endif  // FLOATTYPE_IS_INT
