// Filename: cmath.h
// Created by:  drose (19May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CMATH_H
#define CMATH_H

#include "pandabase.h"

#include <math.h>

// This file declares a number of C++-style overloading wrappers
// around the standard math library functions, so we can use
// overloading to differentiate on type instead of having to know
// explicitly whether we need to call, for instance, sqrtf() or
// sqrt().

INLINE_LINMATH float csqrt(float v);
INLINE_LINMATH float csin(float v);
INLINE_LINMATH float ccos(float v);
INLINE_LINMATH float ctan(float v);
INLINE_LINMATH void  csincos(float v, float *pSinResult, float *pCosResult);  // does both at once (faster on x86)
INLINE_LINMATH float cabs(float v);
INLINE_LINMATH float catan(float v);
INLINE_LINMATH float catan2(float y, float x);
//INLINE_LINMATH float cfloor(float f);
//INLINE_LINMATH float cceil(float f);

INLINE_LINMATH double cfloor(double f);
INLINE_LINMATH double cceil(double f);
INLINE_LINMATH double csqrt(double v);
INLINE_LINMATH double csin(double v);
INLINE_LINMATH double ccos(double v);
INLINE_LINMATH double ctan(double v);
INLINE_LINMATH double cabs(double v);
INLINE_LINMATH double catan(double v);
INLINE_LINMATH double catan2(double y, double x);
INLINE_LINMATH void   csincos(double v, double *pSinResult, double *pCosResult);  // does both at once (faster on x86)

// Returns true if the number is nan, false if it's a genuine number
// or infinity.
INLINE_LINMATH bool cnan(double v);

#include "cmath.I"

#endif

