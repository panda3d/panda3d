/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvec3_ops_src.I
 * @author drose
 * @date 2000-03-08
 */

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3)
operator * (FLOATTYPE scalar, const FLOATNAME(LVecBase3) &a) {
  return a * scalar;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint3)
operator * (FLOATTYPE scalar, const FLOATNAME(LPoint3) &a) {
  return a * scalar;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector3)
operator * (FLOATTYPE scalar, const FLOATNAME(LVector3) &a) {
  return a * scalar;
}

/**
 *
 */

INLINE_LINMATH FLOATTYPE
dot(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b) {
  return a.dot(b);
}

/**
 *
 */

INLINE_LINMATH FLOATNAME(LVecBase3)
cross(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b) {
  return a.cross(b);
}

/**
 *
 */

INLINE_LINMATH FLOATNAME(LVector3)
cross(const FLOATNAME(LVector3) &a, const FLOATNAME(LVector3) &b) {
  return FLOATNAME(LVector3)(a.cross(b));
}

#ifndef FLOATTYPE_IS_INT
/**
 *
 */

INLINE_LINMATH FLOATTYPE
length(const FLOATNAME(LVecBase3) &a) {
  return a.length();
}

/**
 * Returns a normalized vector from the given vector.
 */

INLINE_LINMATH FLOATNAME(LVector3)
normalize(const FLOATNAME(LVecBase3) &v) {
  FLOATNAME(LVector3) v1 = v;
  v1.normalize();
  return v1;
}
#endif  // FLOATTYPE_IS_INT

/**
 * Writes the value to the datagram.
 */
INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LVecBase3) &value) {
  value.write_datagram(dest);
}

/**
 * Reads the value from the datagram.
 */
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LVecBase3) &result, DatagramIterator &source) {
  result.read_datagram(source);
}
