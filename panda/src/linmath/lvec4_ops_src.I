// Filename: lvec4_ops_src.I
// Created by:  drose (08Mar00)
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
//     Function: scalar * LVecBase4
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)
operator * (FLOATTYPE scalar, const FLOATNAME(LVecBase4) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LPoint4
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint4)
operator * (FLOATTYPE scalar, const FLOATNAME(LPoint4) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LVector4
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector4)
operator * (FLOATTYPE scalar, const FLOATNAME(LVector4) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: dot product of LVecBase4
//  Description:
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATTYPE
dot(const FLOATNAME(LVecBase4) &a, const FLOATNAME(LVecBase4) &b) {
  return a.dot(b);
}

////////////////////////////////////////////////////////////////////
//     Function: length of a vector
//  Description:
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATTYPE
length(const FLOATNAME(LVector4) &a) {
  return a.length();
}


////////////////////////////////////////////////////////////////////
//     Function: normalize
//  Description: Returns a normalized vector from the given vector.
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVector4)
normalize(const FLOATNAME(LVector4) &v) {
  FLOATNAME(LVector4) v1 = v;
  v1.normalize();
  return v1;
}

////////////////////////////////////////////////////////////////////
//     Function: generic_write_datagram
//  Description: Writes the value to the datagram.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LVecBase4) &value) {
  value.write_datagram(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: generic_read_datagram
//  Description: Reads the value from the datagram.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LVecBase4) &result, DatagramIterator &source) {
  result.read_datagram(source);
}
