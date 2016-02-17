/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cmath.h
 * @author drose
 * @date 2000-05-19
 */

#ifndef CMATH_H
#define CMATH_H

// This file declares a number of C++-style overloading wrappers around the
// standard math library functions, so we can use overloading to differentiate
// on type instead of having to know explicitly whether we need to call, for
// instance, sqrtf() or sqrt().

#include "dtoolbase.h"

#include <cmath>
#include <cfloat>
#include <limits>

INLINE float csqrt(float v);
INLINE float csin(float v);
INLINE float ccos(float v);
INLINE float ctan(float v);
INLINE void csincos(float v, float *sin_result, float *cos_result);
INLINE float csin_over_x(float v);
INLINE float cabs(float v);
INLINE float catan(float v);
INLINE float catan2(float y, float x);
INLINE float casin(float v);
INLINE float cacos(float v);
INLINE float cmod(float x, float y);
INLINE float cpow(float x, float y);

INLINE double cfloor(double f);
INLINE double cceil(double f);
INLINE double cfrac(double f);
INLINE double csqrt(double v);
INLINE double csin(double v);
INLINE double ccos(double v);
INLINE double ctan(double v);
INLINE void csincos(double v, double *sin_result, double *cos_result);
INLINE double cabs(double v);
INLINE double catan(double v);
INLINE double catan2(double y, double x);
INLINE double casin(double v);
INLINE double cacos(double v);
INLINE double cmod(double x, double y);
INLINE double cpow(double x, double y);

INLINE int cpow(int x, int y);

// Returns true if the number is NaN, false if it's a genuine number or
// infinity.
INLINE bool cnan(float v);
INLINE bool cnan(double v);

// Returns true if the number is infinity.
INLINE bool cinf(float v);
INLINE bool cinf(double v);

// Return NaN and infinity, respectively.
INLINE float make_nan(float);
INLINE double make_nan(double);
INLINE float make_inf(float);
INLINE double make_inf(double);

INLINE int cmod(int x, int y);

#include "cmath.I"

#endif
