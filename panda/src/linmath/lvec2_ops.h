// Filename: lvec2_ops.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVEC2_OPS_H
#define LVEC2_OPS_H

#include "lvecBase2.h"
#include "lpoint2.h"
#include "lvector2.h"

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)
template<class NumType, class NumType2>
INLINE LVecBase2<NumType> 
operator * (NumType2 scalar, const LVecBase2<NumType> &a);

template<class NumType, class NumType2>
INLINE LPoint2<NumType> 
operator * (NumType2 scalar, const LPoint2<NumType> &a);

template<class NumType, class NumType2>
INLINE LVector2<NumType> 
operator * (NumType2 scalar, const LVector2<NumType> &a);


// dot product
template<class NumType>
INLINE NumType
dot(const LVecBase2<NumType> &a, const LVecBase2<NumType> &b);


// Length of a vector.
template<class NumType>
INLINE NumType
length(const LVector2<NumType> &a);


// A normalized vector.
template<class NumType>
INLINE LVector2<NumType>
normalize(const LVector2<NumType> &v);



#include "lvec2_ops.I"

#endif
