/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvec3_ops_src.h
 * @author drose
 * @date 2000-03-08
 */

// When possible, operators have been defined within the classes.  This file
// defines operator functions outside of classes where necessary.  It also
// defines some convenient out-of-class wrappers around in-class functions
// (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)
INLINE_LINMATH FLOATNAME(LVecBase3)
operator * (FLOATTYPE scalar, const FLOATNAME(LVecBase3) &a);

INLINE_LINMATH FLOATNAME(LPoint3)
operator * (FLOATTYPE scalar, const FLOATNAME(LPoint3) &a);

INLINE_LINMATH FLOATNAME(LVector3)
operator * (FLOATTYPE scalar, const FLOATNAME(LVector3) &a);


// dot product
INLINE_LINMATH FLOATTYPE
dot(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b);


// cross product
INLINE_LINMATH FLOATNAME(LVecBase3)
cross(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b);

INLINE_LINMATH FLOATNAME(LVector3)
cross(const FLOATNAME(LVector3) &a, const FLOATNAME(LVector3) &b);

#ifndef FLOATTYPE_IS_INT
// Length of a vector.
INLINE_LINMATH FLOATTYPE
length(const FLOATNAME(LVecBase3) &a);

// A normalized vector.
INLINE_LINMATH FLOATNAME(LVector3)
normalize(const FLOATNAME(LVecBase3) &v);
#endif  // FLOATTYPE_IS_INT

INLINE_LINMATH void
generic_write_datagram(Datagram &dest, const FLOATNAME(LVecBase3) &value);
INLINE_LINMATH void
generic_read_datagram(FLOATNAME(LVecBase3) &result, DatagramIterator &source);

#include "lvec3_ops_src.I"
