// Filename: lvec4_ops_src.h
// Created by:  drose (08Mar00)
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


// Length of a vector.
INLINE_LINMATH FLOATTYPE
length(const FLOATNAME(LVector4) &a);


// A normalized vector.
INLINE_LINMATH FLOATNAME(LVector4)
normalize(const FLOATNAME(LVector4) &v);


#include "lvec4_ops_src.I"
