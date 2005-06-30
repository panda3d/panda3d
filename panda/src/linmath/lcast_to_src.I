// Filename: lcast_to_src.I
// Created by:  
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

INLINE_LINMATH FLOATNAME2(LVecBase2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase2) &source) {
  return FLOATNAME2(LVecBase2)(source[0], source[1]);
}

INLINE_LINMATH FLOATNAME2(LVecBase3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase3) &source) {
  return FLOATNAME2(LVecBase3)(source[0], source[1], source[2]);
}

INLINE_LINMATH FLOATNAME2(LVecBase4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase4) &source) {
  return FLOATNAME2(LVecBase4)(source[0], source[1], source[2], source[3]);
}

INLINE_LINMATH FLOATNAME2(LVector2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector2) &source) {
  return FLOATNAME2(LVector2)(source[0], source[1]);
}

INLINE_LINMATH FLOATNAME2(LVector3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector3) &source) {
  return FLOATNAME2(LVector3)(source[0], source[1], source[2]);
}

INLINE_LINMATH FLOATNAME2(LVector4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector4) &source) {
  return FLOATNAME2(LVector4)(source[0], source[1], source[2], source[3]);
}

INLINE_LINMATH FLOATNAME2(LPoint2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint2) &source) {
  return FLOATNAME2(LPoint2)(source[0], source[1]);
}

INLINE_LINMATH FLOATNAME2(LPoint3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint3) &source) {
  return FLOATNAME2(LPoint3)(source[0], source[1], source[2]);
}

INLINE_LINMATH FLOATNAME2(LPoint4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint4) &source) {
  return FLOATNAME2(LPoint4)(source[0], source[1], source[2], source[3]);
}

INLINE_LINMATH FLOATNAME2(LQuaternion)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LQuaternion) &c) {
  return FLOATNAME2(LQuaternion)(c.get_r(), c.get_i(), c.get_j(), c.get_k());
}

INLINE_LINMATH FLOATNAME2(LMatrix3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix3) &source) {
  return FLOATNAME2(LMatrix3)
    (source(0, 0), source(0, 1), source(0, 2),
     source(1, 0), source(1, 1), source(1, 2),
     source(2, 0), source(2, 1), source(2, 2));
}

INLINE_LINMATH FLOATNAME2(LMatrix4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix4) &source) {
  return FLOATNAME2(LMatrix4)
    (source(0, 0), source(0, 1), source(0, 2), source(0, 3),
     source(1, 0), source(1, 1), source(1, 2), source(1, 3),
     source(2, 0), source(2, 1), source(2, 2), source(2, 3),
     source(3, 0), source(3, 1), source(3, 2), source(3, 3));
}
