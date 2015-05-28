// Filename: lcast_to_src.h
// Created by:  drose (03Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CPPPARSER

INLINE_LINMATH const FLOATNAME(LVecBase2) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVecBase2) &source);

INLINE_LINMATH const FLOATNAME(LVecBase3) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVecBase3) &source);

INLINE_LINMATH const FLOATNAME(LVecBase4) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVecBase4) &source);

INLINE_LINMATH const FLOATNAME(LVector2) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVector2) &source);

INLINE_LINMATH const FLOATNAME(LVector3) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVector3) &source);

INLINE_LINMATH const FLOATNAME(LVector4) &
lcast_to(FLOATTYPE *, const FLOATNAME(LVector4) &source);

INLINE_LINMATH const FLOATNAME(LPoint2) &
lcast_to(FLOATTYPE *, const FLOATNAME(LPoint2) &source);

INLINE_LINMATH const FLOATNAME(LPoint3) &
lcast_to(FLOATTYPE *, const FLOATNAME(LPoint3) &source);

INLINE_LINMATH const FLOATNAME(LPoint4) &
lcast_to(FLOATTYPE *, const FLOATNAME(LPoint4) &source);

INLINE_LINMATH const FLOATNAME(LQuaternion) &
lcast_to(FLOATTYPE *, const FLOATNAME(LQuaternion) &c);

INLINE_LINMATH const FLOATNAME(LMatrix3) &
lcast_to(FLOATTYPE *, const FLOATNAME(LMatrix3) &source);

INLINE_LINMATH const FLOATNAME(LMatrix4) &
lcast_to(FLOATTYPE *, const FLOATNAME(LMatrix4) &source);

INLINE_LINMATH FLOATNAME2(LVecBase2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase2) &source);

INLINE_LINMATH FLOATNAME2(LVecBase3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase3) &source);

INLINE_LINMATH FLOATNAME2(LVecBase4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase4) &source);

INLINE_LINMATH FLOATNAME2(LVector2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector2) &source);

INLINE_LINMATH FLOATNAME2(LVector3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector3) &source);

INLINE_LINMATH FLOATNAME2(LVector4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector4) &source);

INLINE_LINMATH FLOATNAME2(LPoint2)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint2) &source);

INLINE_LINMATH FLOATNAME2(LPoint3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint3) &source);

INLINE_LINMATH FLOATNAME2(LPoint4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint4) &source);

INLINE_LINMATH FLOATNAME2(LQuaternion)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LQuaternion) &c);

INLINE_LINMATH FLOATNAME2(LMatrix3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix3) &source);

INLINE_LINMATH FLOATNAME2(LMatrix4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix4) &source);

INLINE_LINMATH FLOATNAME2(LVecBase2)
lcast_to(FLOATTYPE2 *, const LVecBase2i &source);

INLINE_LINMATH FLOATNAME2(LVecBase3)
lcast_to(FLOATTYPE2 *, const LVecBase3i &source);

INLINE_LINMATH FLOATNAME2(LVecBase4)
lcast_to(FLOATTYPE2 *, const LVecBase4i &source);

INLINE_LINMATH FLOATNAME2(LVector2)
lcast_to(FLOATTYPE2 *, const LVector2i &source);

INLINE_LINMATH FLOATNAME2(LVector3)
lcast_to(FLOATTYPE2 *, const LVector3i &source);

INLINE_LINMATH FLOATNAME2(LVector4)
lcast_to(FLOATTYPE2 *, const LVector4i &source);

INLINE_LINMATH FLOATNAME2(LPoint2)
lcast_to(FLOATTYPE2 *, const LPoint2i &source);

INLINE_LINMATH FLOATNAME2(LPoint3)
lcast_to(FLOATTYPE2 *, const LPoint3i &source);

INLINE_LINMATH FLOATNAME2(LPoint4)
lcast_to(FLOATTYPE2 *, const LPoint4i &source);

#include "lcast_to_src.I"

#endif  // CPPPARSER
