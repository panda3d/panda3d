/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lpoint2_src.I
 * @author drose
 * @date 2000-03-08
 */

/**
 * Constructs a new LPoint2 from a LVecBase2
 */
INLINE_LINMATH FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(const FLOATNAME(LVecBase2)& copy) :
  FLOATNAME(LVecBase2)(copy)
{
}

/**
 * Constructs a new LPoint2 all components set to the fill value.
 */
INLINE_LINMATH FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase2)(fill_value)
{
}

/**
 * Constructs a new LPoint2 with the given components
 */
INLINE_LINMATH FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(FLOATTYPE x, FLOATTYPE y) :
  FLOATNAME(LVecBase2)(x, y)
{
}

/**
 * Returns a zero-length point.
 */
INLINE_LINMATH const FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
zero() {
  return (const FLOATNAME(LPoint2) &)FLOATNAME(LVecBase2)::zero();
}

/**
 * Returns a unit X point.
 */
INLINE_LINMATH const FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
unit_x() {
  return (const FLOATNAME(LPoint2) &)FLOATNAME(LVecBase2)::unit_x();
}

/**
 * Returns a unit Y point.
 */
INLINE_LINMATH const FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
unit_y() {
  return (const FLOATNAME(LPoint2) &)FLOATNAME(LVecBase2)::unit_y();
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator - () const {
  return FLOATNAME(LVecBase2)::operator - ();
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LPoint2)::
operator + (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator + (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LPoint2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint2)(FLOATNAME(LVecBase2)::operator * (scalar));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint2)(FLOATNAME(LVecBase2)::operator / (scalar));
}

#ifndef FLOATTYPE_IS_INT
/**
 * Normalizes the vector and returns the normalized vector as a copy.  If the
 * vector was a zero-length vector, a zero length vector will be returned.
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
normalized() const {
  return FLOATNAME(LVecBase2)::normalized();
}

/**
 * Returns a new vector representing the projection of this vector onto
 * another one.  The resulting vector will be a scalar multiple of onto.
 */
INLINE_LINMATH FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
project(const FLOATNAME(LVecBase2) &onto) const {
  return FLOATNAME(LVecBase2)::project(onto);
}
#endif  // FLOATTYPE_IS_INT
