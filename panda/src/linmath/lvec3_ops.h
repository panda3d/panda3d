// Filename: lvec3_ops.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVEC3_OPS_H
#define LVEC3_OPS_H

#include "lvecBase3.h"
#include "lpoint3.h"
#include "lvector3.h"

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)
template<class NumType, class NumType2>
INLINE LVecBase3<NumType> 
operator * (NumType2 scalar, const LVecBase3<NumType> &a);

template<class NumType, class NumType2>
INLINE LPoint3<NumType> 
operator * (NumType2 scalar, const LPoint3<NumType> &a);

template<class NumType, class NumType2>
INLINE LVector3<NumType> 
operator * (NumType2 scalar, const LVector3<NumType> &a);


// dot product
template<class NumType>
INLINE NumType
dot(const LVecBase3<NumType> &a, const LVecBase3<NumType> &b);


// cross product
template<class NumType>
INLINE LVecBase3<NumType>
cross(const LVecBase3<NumType> &a, const LVecBase3<NumType> &b);

template<class NumType>
INLINE LVector3<NumType>
cross(const LVector3<NumType> &a, const LVector3<NumType> &b);


// Length of a vector.
template<class NumType>
INLINE NumType
length(const LVector3<NumType> &a);


// A normalized vector.
template<class NumType>
INLINE LVector3<NumType>
normalize(const LVector3<NumType> &v);



#include "lvec3_ops.I"

#endif
