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

INLINE LVecBase2f cast_to_float(const LVecBase2d &source);
INLINE LVecBase3f cast_to_float(const LVecBase3d &source);
INLINE LVecBase4f cast_to_float(const LVecBase4d &source);
INLINE LVector2f cast_to_float(const LVector2d &source);
INLINE LVector3f cast_to_float(const LVector3d &source);
INLINE LVector4f cast_to_float(const LVector4d &source);
INLINE LPoint2f cast_to_float(const LPoint2d &source);
INLINE LPoint3f cast_to_float(const LPoint3d &source);
INLINE LPoint4f cast_to_float(const LPoint4d &source);
INLINE LMatrix3f cast_to_float(const LMatrix3d &source);
INLINE LMatrix4f cast_to_float(const LMatrix4d &source);

#include "cast_to_float.I"

#endif


