// Filename: lcast_to_src.h
// Created by:  drose (03Apr01)
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
lcast_to(FLOATTYPE2 *, const FLOATNAME(LQuaternion)& c);

INLINE_LINMATH FLOATNAME2(LMatrix3)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix3) &source);

INLINE_LINMATH FLOATNAME2(LMatrix4)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix4) &source);

#include "lcast_to_src.I"
