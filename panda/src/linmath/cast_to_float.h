// Filename: cast_to_float.h
// Created by:  drose (24May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CAST_TO_FLOAT_H
#define CAST_TO_FLOAT_H

#include "luse.h"

// The functions in this file are primarily for the benefit of a
// higher-level language that can't take advantage of the LCAST macro.
// These are a number of functions that convert our various math
// objects between floats and doubles.

INLINE LVecBase2<float> cast_to_float(const LVecBase2<double> &source);
INLINE LVecBase3<float> cast_to_float(const LVecBase3<double> &source);
INLINE LVecBase4<float> cast_to_float(const LVecBase4<double> &source);
INLINE LVector2<float> cast_to_float(const LVector2<double> &source);
INLINE LVector3<float> cast_to_float(const LVector3<double> &source);
INLINE LVector4<float> cast_to_float(const LVector4<double> &source);
INLINE LPoint2<float> cast_to_float(const LPoint2<double> &source);
INLINE LPoint3<float> cast_to_float(const LPoint3<double> &source);
INLINE LPoint4<float> cast_to_float(const LPoint4<double> &source);
INLINE LMatrix3<float> cast_to_float(const LMatrix3<double> &source);
INLINE LMatrix4<float> cast_to_float(const LMatrix4<double> &source);

#include "cast_to_float.I"

#endif


