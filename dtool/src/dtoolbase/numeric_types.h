// Filename: numeric_types.h
// Created by:  drose (06Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef NUMERIC_TYPES_H
#define NUMERIC_TYPES_H

#include "dtoolbase.h"

// This header file defines a number of typedefs that correspond to
// the various numeric types for unsigned and signed numbers of
// various widths.

#if defined(WIN32_VC) && !defined(CPPPARSER)
typedef signed __int8  PN_int8;
typedef signed __int16 PN_int16;
typedef signed __int32 PN_int32;
typedef signed __int64 PN_int64;

typedef unsigned __int8  PN_uint8;
typedef unsigned __int16 PN_uint16;
typedef unsigned __int32 PN_uint32;
typedef unsigned __int64 PN_uint64;

#else

typedef signed char PN_int8;
typedef short int PN_int16;
typedef int PN_int32;
typedef long long int PN_int64;

typedef unsigned char PN_uint8;
typedef unsigned short int PN_uint16;
typedef unsigned int PN_uint32;
typedef unsigned long long int PN_uint64;

#endif

typedef double PN_float64;
typedef float PN_float32;

#ifndef STDFLOAT_DOUBLE
// The default setting--single-precision floats.
typedef float PN_stdfloat;
#else  // STDFLOAT_DOUBLE
// The specialty setting--double-precision floats.
typedef double PN_stdfloat;
#endif  // STDFLOAT_DOUBLE

#endif

