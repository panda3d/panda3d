// Filename: lvec2_ops_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)
INLINE FLOATNAME(LVecBase2) 
operator * (FLOATTYPE scalar, const FLOATNAME(LVecBase2) &a);

INLINE FLOATNAME(LPoint2) 
operator * (FLOATTYPE scalar, const FLOATNAME(LPoint2) &a);

INLINE FLOATNAME(LVector2) 
operator * (FLOATTYPE scalar, const FLOATNAME(LVector2) &a);


// dot product
INLINE FLOATTYPE
dot(const FLOATNAME(LVecBase2) &a, const FLOATNAME(LVecBase2) &b);

// Length of a vector.
INLINE FLOATTYPE
length(const FLOATNAME(LVector2) &a);


// A normalized vector.
INLINE FLOATNAME(LVector2)
normalize(const FLOATNAME(LVector2) &v);


#include "lvec2_ops_src.I"



