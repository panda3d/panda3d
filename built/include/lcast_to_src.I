/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lcast_to_src.I
 */

INLINE_LINMATH const FLOATNAME(LVecBase2) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVecBase2) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LVecBase3) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVecBase3) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LVecBase4) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVecBase4) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LVector2) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVector2) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LVector3) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVector3) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LVector4) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVector4) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LPoint2) &
lcast_to(FLOATTYPE *, const FLOATNAME(LPoint2) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LPoint3) &
lcast_to(FLOATTYPE *, const FLOATNAME(LPoint3) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LPoint4) &
lcast_to(FLOATTYPE *, const FLOATNAME(LPoint4) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LQuaternion) &
lcast_to(FLOATTYPE *, const FLOATNAME(LQuaternion) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LMatrix3) &
lcast_to(FLOATTYPE *, const FLOATNAME(LMatrix3) &source) {
  return source;
}

INLINE_LINMATH const FLOATNAME(LMatrix4) &
lcast_to(FLOATTYPE *, const FLOATNAME(LMatrix4) &source) {
  return source;
}

INLINE_LINMATH FLOATNAME2(LVecBase2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase2) &source) {
  return FLOATNAME2(LVecBase2)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1]);
}

INLINE_LINMATH FLOATNAME2(LVecBase3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase3) &source) {
  return FLOATNAME2(LVecBase3)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2]);
}

INLINE_LINMATH FLOATNAME2(LVecBase4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase4) &source) {
  return FLOATNAME2(LVecBase4)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2], (FLOATTYPE2)source[3]);
}

INLINE_LINMATH FLOATNAME2(LVector2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector2) &source) {
  return FLOATNAME2(LVector2)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1]);
}

INLINE_LINMATH FLOATNAME2(LVector3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector3) &source) {
  return FLOATNAME2(LVector3)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2]);
}

INLINE_LINMATH FLOATNAME2(LVector4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector4) &source) {
  return FLOATNAME2(LVector4)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2], (FLOATTYPE2)source[3]);
}

INLINE_LINMATH FLOATNAME2(LPoint2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint2) &source) {
  return FLOATNAME2(LPoint2)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1]);
}

INLINE_LINMATH FLOATNAME2(LPoint3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint3) &source) {
  return FLOATNAME2(LPoint3)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2]);
}

INLINE_LINMATH FLOATNAME2(LPoint4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint4) &source) {
  return FLOATNAME2(LPoint4)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2], (FLOATTYPE2)source[3]);
}

INLINE_LINMATH FLOATNAME2(LQuaternion)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LQuaternion) &c) {
  return FLOATNAME2(LQuaternion)((FLOATTYPE2)c.get_r(), (FLOATTYPE2)c.get_i(), (FLOATTYPE2)c.get_j(), (FLOATTYPE2)c.get_k());
}

INLINE_LINMATH FLOATNAME2(LMatrix3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix3) &source) {
  return FLOATNAME2(LMatrix3)
    ((FLOATTYPE2)source(0, 0), (FLOATTYPE2)source(0, 1), (FLOATTYPE2)source(0, 2),
     (FLOATTYPE2)source(1, 0), (FLOATTYPE2)source(1, 1), (FLOATTYPE2)source(1, 2),
     (FLOATTYPE2)source(2, 0), (FLOATTYPE2)source(2, 1), (FLOATTYPE2)source(2, 2));
}

INLINE_LINMATH FLOATNAME2(LMatrix4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix4) &source) {
  return FLOATNAME2(LMatrix4)
    ((FLOATTYPE2)source(0, 0), (FLOATTYPE2)source(0, 1), (FLOATTYPE2)source(0, 2), (FLOATTYPE2)source(0, 3),
     (FLOATTYPE2)source(1, 0), (FLOATTYPE2)source(1, 1), (FLOATTYPE2)source(1, 2), (FLOATTYPE2)source(1, 3),
     (FLOATTYPE2)source(2, 0), (FLOATTYPE2)source(2, 1), (FLOATTYPE2)source(2, 2), (FLOATTYPE2)source(2, 3),
     (FLOATTYPE2)source(3, 0), (FLOATTYPE2)source(3, 1), (FLOATTYPE2)source(3, 2), (FLOATTYPE2)source(3, 3));
}

INLINE_LINMATH FLOATNAME2(LVecBase2)
lcast_to(FLOATTYPE2 *, const LVecBase2i &source) {
  return FLOATNAME2(LVecBase2)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1]);
}

INLINE_LINMATH FLOATNAME2(LVecBase3)
lcast_to(FLOATTYPE2 *, const LVecBase3i &source) {
  return FLOATNAME2(LVecBase3)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2]);
}

INLINE_LINMATH FLOATNAME2(LVecBase4)
lcast_to(FLOATTYPE2 *, const LVecBase4i &source) {
  return FLOATNAME2(LVecBase4)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2], (FLOATTYPE2)source[3]);
}

INLINE_LINMATH FLOATNAME2(LVector2)
lcast_to(FLOATTYPE2 *, const LVector2i &source) {
  return FLOATNAME2(LVector2)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1]);
}

INLINE_LINMATH FLOATNAME2(LVector3)
lcast_to(FLOATTYPE2 *, const LVector3i &source) {
  return FLOATNAME2(LVector3)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2]);
}

INLINE_LINMATH FLOATNAME2(LVector4)
lcast_to(FLOATTYPE2 *, const LVector4i &source) {
  return FLOATNAME2(LVector4)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2], (FLOATTYPE2)source[3]);
}

INLINE_LINMATH FLOATNAME2(LPoint2)
lcast_to(FLOATTYPE2 *, const LPoint2i &source) {
  return FLOATNAME2(LPoint2)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1]);
}

INLINE_LINMATH FLOATNAME2(LPoint3)
lcast_to(FLOATTYPE2 *, const LPoint3i &source) {
  return FLOATNAME2(LPoint3)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2]);
}

INLINE_LINMATH FLOATNAME2(LPoint4)
lcast_to(FLOATTYPE2 *, const LPoint4i &source) {
  return FLOATNAME2(LPoint4)((FLOATTYPE2)source[0], (FLOATTYPE2)source[1], (FLOATTYPE2)source[2], (FLOATTYPE2)source[3]);
}
