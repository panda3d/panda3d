// Filename: lmat_ops_src.I
// Created by:  drose (08Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: LVecBase3 times LMatrix3
//  Description:
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVecBase3)
operator * (const FLOATNAME(LVecBase3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

INLINE_LINMATH FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

INLINE_LINMATH FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2 times LMatrix3
//  Description:
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVector2)
operator * (const FLOATNAME(LVector2) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform_vec(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2 times LMatrix3
//  Description:
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LPoint2)
operator * (const FLOATNAME(LPoint2) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform_point(v);
}


////////////////////////////////////////////////////////////////////
//     Function: LVecBase4 times LMatrix4
//  Description:
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVecBase4)
operator * (const FLOATNAME(LVecBase4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}

INLINE_LINMATH FLOATNAME(LVector4)
operator * (const FLOATNAME(LVector4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}

INLINE_LINMATH FLOATNAME(LPoint4)
operator * (const FLOATNAME(LPoint4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}


////////////////////////////////////////////////////////////////////
//     Function: LVector3 times LMatrix4
//  Description:
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform_vec(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3 times LMatrix4
//  Description:
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform_point(v);
}


////////////////////////////////////////////////////////////////////
//     Function: generic_write_datagram
//  Description: Writes the value to the datagram.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LMatrix3) &value) {
  value.write_datagram(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: generic_read_datagram
//  Description: Reads the value from the datagram.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LMatrix3) &result, DatagramIterator &source) {
  result.read_datagram(source);
}

////////////////////////////////////////////////////////////////////
//     Function: generic_write_datagram
//  Description: Writes the value to the datagram.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LMatrix4) &value) {
  value.write_datagram(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: generic_read_datagram
//  Description: Reads the value from the datagram.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LMatrix4) &result, DatagramIterator &source) {
  result.read_datagram(source);
}
