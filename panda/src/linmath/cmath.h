// Filename: cmath.h
// Created by:  drose (19May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CMATH_H
#define CMATH_H

#include <math.h>

// This file declares a number of C++-style overloading wrappers
// around the standard math library functions, so we can use
// overloading to differentiate on type instead of having to know
// explicitly whether we need to call, for instance, sqrtf() or
// sqrt().

INLINE float csqrt(float v);
INLINE float csin(float v);
INLINE float ccos(float v);
INLINE float cabs(float v);

INLINE double csqrt(double v);
INLINE double csin(double v);
INLINE double ccos(double v);
INLINE double cabs(double v);

// Returns true if the number is nan, false if it's a genuine number
// or infinity.
INLINE bool cnan(double v);

#include "cmath.I"

#endif

