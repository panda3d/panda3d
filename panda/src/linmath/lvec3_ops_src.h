// Filename: lvec3_ops_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)
INLINE FLOATNAME(LVecBase3) 
operator * (FLOATTYPE scalar, const FLOATNAME(LVecBase3) &a);

INLINE FLOATNAME(LPoint3) 
operator * (FLOATTYPE scalar, const FLOATNAME(LPoint3) &a);

INLINE FLOATNAME(LVector3) 
operator * (FLOATTYPE scalar, const FLOATNAME(LVector3) &a);


// dot product
INLINE FLOATTYPE
dot(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b);


// cross product
INLINE FLOATNAME(LVecBase3)
cross(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b);

INLINE FLOATNAME(LVector3)
cross(const FLOATNAME(LVector3) &a, const FLOATNAME(LVector3) &b);


// Length of a vector.
INLINE FLOATTYPE
length(const FLOATNAME(LVector3) &a);

// A normalized vector.
INLINE FLOATNAME(LVector3)
normalize(const FLOATNAME(LVector3) &v);

#include "lvec3_ops_src.I"
