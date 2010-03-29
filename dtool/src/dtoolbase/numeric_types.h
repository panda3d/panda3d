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

#ifdef WIN32_VC
typedef signed __int8  PN_int8;
typedef signed __int16 PN_int16;
typedef signed __int32 PN_int32;
typedef signed __int64 PN_int64;

typedef unsigned __int8  PN_uint8;
typedef unsigned __int16 PN_uint16;
typedef unsigned __int32 PN_uint32;
typedef unsigned __int64 PN_uint64;

#elif defined(PHAVE_STDINT_H)

#include <stdint.h>

typedef int8_t  PN_int8;
typedef int16_t PN_int16;
typedef int32_t PN_int32;
typedef int64_t PN_int64;

typedef uint8_t  PN_uint8;
typedef uint16_t PN_uint16;
typedef uint32_t PN_uint32;
typedef uint64_t PN_uint64;

#else

// This is risky, but we have no other choice.
typedef signed char PN_int8;
typedef short int PN_int16;
typedef int PN_int32;
#if NATIVE_WORDSIZE == 64
typedef long int PN_int64;
#else
typedef long long int PN_int64;
#endif

typedef unsigned char PN_uint8;
typedef unsigned short int PN_uint16;
typedef unsigned int PN_uint32;
#if NATIVE_WORDSIZE == 64
typedef unsigned long int PN_uint64;
#else
typedef unsigned long long int PN_uint64;
#endif

#endif

typedef double PN_float64;
typedef float PN_float32;

#endif

