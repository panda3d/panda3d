/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmat_ops_src.I
 * @author drose
 * @date 2000-03-08
 */

/**
 *
 */

INLINE_LINMATH FLOATNAME(LVecBase3)
operator * (const FLOATNAME(LVecBase3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}
INLINE_LINMATH void
operator *= (FLOATNAME(LVecBase3) &v, const FLOATNAME(LMatrix3) &m) {
  m.xform_in_place(v);
}

INLINE_LINMATH FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}
INLINE_LINMATH void
operator *= (FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix3) &m) {
  m.xform_in_place(v);
}

INLINE_LINMATH FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}
INLINE_LINMATH void
operator *= (FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix3) &m) {
  m.xform_in_place(v);
}

/**
 *
 */

INLINE_LINMATH FLOATNAME(LVector2)
operator * (const FLOATNAME(LVector2) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform_vec(v);
}
INLINE_LINMATH void
operator *= (FLOATNAME(LVector2) &v, const FLOATNAME(LMatrix3) &m) {
  m.xform_vec_in_place(v);
}

/**
 *
 */

INLINE_LINMATH FLOATNAME(LPoint2)
operator * (const FLOATNAME(LPoint2) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform_point(v);
}
INLINE_LINMATH void
operator *= (FLOATNAME(LPoint2) &v, const FLOATNAME(LMatrix3) &m) {
  m.xform_point_in_place(v);
}


/**
 *
 */

INLINE_LINMATH FLOATNAME(LVecBase4)
operator * (const FLOATNAME(LVecBase4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}
INLINE_LINMATH void
operator *= (FLOATNAME(LVecBase4) &v, const FLOATNAME(LMatrix4) &m) {
  m.xform_in_place(v);
}

INLINE_LINMATH FLOATNAME(LVector4)
operator * (const FLOATNAME(LVector4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}
INLINE_LINMATH FLOATNAME(LPoint4)
operator * (const FLOATNAME(LPoint4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}


/**
 *
 */

INLINE_LINMATH FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform_vec(v);
}
INLINE_LINMATH void
operator *= (FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix4) &m) {
  m.xform_vec_in_place(v);
}

/**
 *
 */

INLINE_LINMATH FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform_point(v);
}
INLINE_LINMATH void
operator *= (FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix4) &m) {
  m.xform_point_in_place(v);
}


/**
 * Writes the value to the datagram.
 */
INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LMatrix3) &value) {
  value.write_datagram(dest);
}

/**
 * Reads the value from the datagram.
 */
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LMatrix3) &result, DatagramIterator &source) {
  result.read_datagram(source);
}

/**
 * Writes the value to the datagram.
 */
INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LMatrix4) &value) {
  value.write_datagram(dest);
}

/**
 * Reads the value from the datagram.
 */
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LMatrix4) &result, DatagramIterator &source) {
  result.read_datagram(source);
}
