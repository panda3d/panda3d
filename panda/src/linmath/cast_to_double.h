// Filename: cast_to_double.h
// Created by:  drose (24May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CAST_TO_DOUBLE_H
#define CAST_TO_DOUBLE_H

#include "luse.h"

// The functions in this file are primarily for the benefit of a
// higher-level language that can't take advantage of the LCAST macro.
// These are a number of functions that convert our various math
// objects between floats and doubles.

INLINE LVecBase2<double> cast_to_double(const LVecBase2<float> &source);
INLINE LVecBase3<double> cast_to_double(const LVecBase3<float> &source);
INLINE LVecBase4<double> cast_to_double(const LVecBase4<float> &source);
INLINE LVector2<double> cast_to_double(const LVector2<float> &source);
INLINE LVector3<double> cast_to_double(const LVector3<float> &source);
INLINE LVector4<double> cast_to_double(const LVector4<float> &source);
INLINE LPoint2<double> cast_to_double(const LPoint2<float> &source);
INLINE LPoint3<double> cast_to_double(const LPoint3<float> &source);
INLINE LPoint4<double> cast_to_double(const LPoint4<float> &source);
INLINE LMatrix3<double> cast_to_double(const LMatrix3<float> &source);
INLINE LMatrix4<double> cast_to_double(const LMatrix4<float> &source);

#include "cast_to_double.I"

#endif


