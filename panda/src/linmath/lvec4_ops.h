// Filename: lvec4_ops.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVEC4_OPS_H
#define LVEC4_OPS_H

#include "lvecBase4.h"
#include "lpoint4.h"
#include "lvector4.h"

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)
template<class NumType, class NumType2>
INLINE LVecBase4<NumType> 
operator * (NumType2 scalar, const LVecBase4<NumType> &a);

template<class NumType, class NumType2>
INLINE LPoint4<NumType> 
operator * (NumType2 scalar, const LPoint4<NumType> &a);

template<class NumType, class NumType2>
INLINE LVector4<NumType> 
operator * (NumType2 scalar, const LVector4<NumType> &a);


// dot product
template<class NumType>
INLINE NumType
dot(const LVecBase4<NumType> &a, const LVecBase4<NumType> &b);


// Length of a vector.
template<class NumType>
INLINE NumType
length(const LVector4<NumType> &a);


// A normalized vector.
template<class NumType>
INLINE LVector4<NumType>
normalize(const LVector4<NumType> &v);



#include "lvec4_ops.I"

#endif
