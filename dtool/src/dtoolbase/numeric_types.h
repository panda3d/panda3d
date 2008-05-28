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

#if defined(_LP64)
// A 64-bit environment.

typedef signed char PN_int8;
typedef short PN_int16;
typedef int PN_int32;

typedef unsigned char PN_uint8;
typedef unsigned short PN_uint16;
typedef unsigned int PN_uint32;
typedef long PN_int64;
typedef unsigned long PN_uint64;

typedef double PN_float64;
typedef float PN_float32;

#else  // _LP64
// A 32-bit environment.

typedef signed char PN_int8;
typedef short PN_int16;
typedef int PN_int32;

typedef unsigned char PN_uint8;
typedef unsigned short PN_uint16;
typedef unsigned int PN_uint32;

#ifdef WIN32_VC
typedef __int64 PN_int64;
typedef unsigned __int64 PN_uint64;
#else
typedef long long PN_int64;
typedef unsigned long long PN_uint64;
#endif

typedef double PN_float64;
typedef float PN_float32;

#endif  // _LP64
#endif




