// Filename: cmath.h
// Created by:  drose (19May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CMATH_H
#define CMATH_H

#include "dtoolbase.h"

#include <math.h>

// This file declares a number of C++-style overloading wrappers
// around the standard math library functions, so we can use
// overloading to differentiate on type instead of having to know
// explicitly whether we need to call, for instance, sqrtf() or
// sqrt().

INLINE float csqrt(float v);
INLINE float csin(float v);
INLINE float ccos(float v);
INLINE float ctan(float v);
INLINE void  csincos(float v, float *pSinResult, float *pCosResult);  // does both at once (faster on x86)
INLINE float cabs(float v);
INLINE float catan(float v);
INLINE float catan2(float y, float x);
//INLINE float cfloor(float f);
//INLINE float cceil(float f);

INLINE double cfloor(double f);
INLINE double cceil(double f);
INLINE double csqrt(double v);
INLINE double csin(double v);
INLINE double ccos(double v);
INLINE double ctan(double v);
INLINE double cabs(double v);
INLINE double catan(double v);
INLINE double catan2(double y, double x);
INLINE void   csincos(double v, double *pSinResult, double *pCosResult);  // does both at once (faster on x86)

// Returns true if the number is nan, false if it's a genuine number
// or infinity.
INLINE bool cnan(double v);

#include "cmath.I"

#endif

