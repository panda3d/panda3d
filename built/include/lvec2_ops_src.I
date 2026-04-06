/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvec2_ops_src.I
 * @author drose
 * @date 2000-03-08
 */

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase2)
operator * (FLOATTYPE scalar, const FLOATNAME(LVecBase2) &a) {
  return a * scalar;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LPoint2)
operator * (FLOATTYPE scalar, const FLOATNAME(LPoint2) &a) {
  return a * scalar;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVector2)
operator * (FLOATTYPE scalar, const FLOATNAME(LVector2) &a) {
  return a * scalar;
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE
dot(const FLOATNAME(LVecBase2) &a, const FLOATNAME(LVecBase2) &b) {
  return a.dot(b);
}

#ifndef FLOATTYPE_IS_INT
/**
 *
 */
INLINE_LINMATH FLOATTYPE
length(const FLOATNAME(LVector2) &a) {
  return a.length();
}

/**
 * Returns a normalized vector from the given vector.
 */
INLINE_LINMATH FLOATNAME(LVector2)
normalize(const FLOATNAME(LVector2) &v) {
  FLOATNAME(LVector2) v1 = v;
  v1.normalize();
  return v1;
}
#endif  // FLOATTYPE_IS_INT

/**
 * Writes the value to the datagram.
 */
INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LVecBase2) &value) {
  value.write_datagram(dest);
}

/**
 * Reads the value from the datagram.
 */
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LVecBase2) &result, DatagramIterator &source) {
  result.read_datagram(source);
}
