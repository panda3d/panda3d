// Filename: numeric_types.h
// Created by:  drose (06Jun00)
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

#ifndef NUMERIC_TYPES_H
#define NUMERIC_TYPES_H

// This header file defines a number of typedefs that correspond to
// the various numeric types for unsigned and signed numbers of
// various widths.

// At the present, we use the logic in NSPR to determine this for us.
// Later (especially for non-NSPR platforms) we'll have to do the work
// ourselves.


#ifdef HAVE_NSPR

#include <prtypes.h>

typedef PRInt8 PN_int8;
typedef PRInt16 PN_int16;
typedef PRInt32 PN_int32;
typedef PRInt64 PN_int64;

typedef PRUint8 PN_uint8;
typedef PRUint16 PN_uint16;
typedef PRUint32 PN_uint32;
typedef PRUint64 PN_uint64;

typedef PRFloat64 PN_float64;

// NSPR doesn't define a float32.
typedef float PN_float32;

#else // HAVE_NSPR

// Without NSPR, and without any other information, we need some
// fallback.  For now, we'll just assume a typical 32-bit environment.

typedef signed char PN_int8;
typedef short PN_int16;
typedef long PN_int32;

#ifdef WIN32_VC
typedef __int64 PN_int64;
typedef unsigned __int64 PN_uint64;
#else
typedef long long PN_int64;
typedef unsigned long long PN_uint64;
#endif

typedef unsigned char PN_uint8;
typedef unsigned short PN_uint16;
typedef unsigned long PN_uint32;

typedef double PN_float64;
typedef float PN_float32;

#endif  // HAVE_NSPR

#endif




