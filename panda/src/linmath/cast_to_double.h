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

INLINE LVecBase2d cast_to_double(const LVecBase2f &source);
INLINE LVecBase3d cast_to_double(const LVecBase3f &source);
INLINE LVecBase4d cast_to_double(const LVecBase4f &source);
INLINE LVector2d cast_to_double(const LVector2f &source);
INLINE LVector3d cast_to_double(const LVector3f &source);
INLINE LVector4d cast_to_double(const LVector4f &source);
INLINE LPoint2d cast_to_double(const LPoint2f &source);
INLINE LPoint3d cast_to_double(const LPoint3f &source);
INLINE LPoint4d cast_to_double(const LPoint4f &source);
INLINE LMatrix3d cast_to_double(const LMatrix3f &source);
INLINE LMatrix4d cast_to_double(const LMatrix4f &source);

#include "cast_to_double.I"

#endif


