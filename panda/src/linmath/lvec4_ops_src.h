// Filename: lvec4_ops_src.h
// Created by:  drose (08Mar00)
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

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)
INLINE_LINMATH FLOATNAME(LVecBase4)
operator * (FLOATTYPE scalar, const FLOATNAME(LVecBase4) &a);

INLINE_LINMATH FLOATNAME(LPoint4)
operator * (FLOATTYPE scalar, const FLOATNAME(LPoint4) &a);

INLINE_LINMATH FLOATNAME(LVector4)
operator * (FLOATTYPE scalar, const FLOATNAME(LVector4) &a);


// dot product
INLINE_LINMATH FLOATTYPE
dot(const FLOATNAME(LVecBase4) &a, const FLOATNAME(LVecBase4) &b);

#ifndef FLOATTYPE_IS_INT
// Length of a vector.
INLINE_LINMATH FLOATTYPE
length(const FLOATNAME(LVector4) &a);

// A normalized vector.
INLINE_LINMATH FLOATNAME(LVector4)
normalize(const FLOATNAME(LVector4) &v);
#endif  // FLOATTYPE_IS_INT

INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LVecBase4) &value);
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LVecBase4) &result, DatagramIterator &source);


#include "lvec4_ops_src.I"
