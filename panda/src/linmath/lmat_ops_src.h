// Filename: lmat_ops_src.h
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

BEGIN_PUBLISH

// vector times matrix3

INLINE_LINMATH FLOATNAME(LVecBase3)
operator * (const FLOATNAME(LVecBase3) &v, const FLOATNAME(LMatrix3) &m);

INLINE_LINMATH FLOATNAME(LVector2)
operator * (const FLOATNAME(LVector2) &v, const FLOATNAME(LMatrix3) &m);

INLINE_LINMATH FLOATNAME(LPoint2)
operator * (const FLOATNAME(LPoint2) &v, const FLOATNAME(LMatrix3) &m);


// vector times matrix4

INLINE_LINMATH FLOATNAME(LVecBase4)
operator * (const FLOATNAME(LVecBase4) &v, const FLOATNAME(LMatrix4) &m);

INLINE_LINMATH FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix4) &m);

INLINE_LINMATH FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix4) &m);

INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LMatrix3) &value);
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LMatrix3) &result, DatagramIterator &source);
INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LMatrix4) &value);
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LMatrix4) &result, DatagramIterator &source);

END_PUBLISH

#include "lmat_ops_src.I"
