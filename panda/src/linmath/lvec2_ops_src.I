// Filename: lvec2_ops_src.I
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
//     Function: scalar * LVecBase2
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2)
operator * (FLOATTYPE scalar, const FLOATNAME(LVecBase2) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LPoint2
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint2)
operator * (FLOATTYPE scalar, const FLOATNAME(LPoint2) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LVector2
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2)
operator * (FLOATTYPE scalar, const FLOATNAME(LVector2) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: dot product of LVecBase2
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE
dot(const FLOATNAME(LVecBase2) &a, const FLOATNAME(LVecBase2) &b) {
  return a.dot(b);
}

////////////////////////////////////////////////////////////////////
//     Function: length of a vector
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE
length(const FLOATNAME(LVector2) &a) {
  return a.length();
}

////////////////////////////////////////////////////////////////////
//     Function: normalize
//  Description: Returns a normalized vector from the given vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector2)
normalize(const FLOATNAME(LVector2) &v) {
  FLOATNAME(LVector2) v1 = v;
  v1.normalize();
  return v1;
}
