/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMFloat.h
	The file containing functions and constants for floating point values.
 */

#ifndef _FM_FLOAT_H_
#define _FM_FLOAT_H_

#if !defined(_INC_FLOAT) && (defined (WIN32) || defined (LINUX) || defined(__APPLE__))
#include <float.h>
#endif // _INC_FLOAT, WIN32 and LINUX

/** The default tolerance for double-sized floating-point comparison functions. */
#define DBL_TOLERANCE 0.0001
/** The default tolerance for single-sized floating-point comparison functions. */
#define FLT_TOLERANCE 0.0001f

/** Returns whether two floating-point values are equivalent within a given tolerance.
	@param f1 A first floating-point value.
	@param f2 A second floating-point value. */
inline bool IsEquivalent(float f1, float f2) { return f1 - f2 < FLT_TOLERANCE && f2 - f1 < FLT_TOLERANCE; }

/** Returns whether two floating-point values are equivalent within a given tolerance.
	@param f1 A first floating-point value.
	@param f2 A second floating-point value.
	@param tolerance The tolerance in which to accept the two floating-point values as equivalent. */
inline bool IsEquivalent(float f1, float f2, float tolerance) { return f1 - f2 < tolerance && f2 - f1 < tolerance; }

/** Returns whether two double-sized floating-point values are equivalent.
	@param f1 A first double-sized floating-point value.
	@param f2 A second double-sized floating-point value. */
inline bool IsEquivalent(double f1, double f2) { return f1 - f2 < DBL_TOLERANCE && f2 - f1 < DBL_TOLERANCE; }

/** Returns whether two double-sized floating-point values are equivalent within a given tolerance.
	@param f1 A first double-sized floating-point value.
	@param f2 A second double-sized floating-point value.
	@param tolerance The tolerance in which to accept the two double-sized floating-point values as equivalent. */
inline bool IsEquivalent(double f1, double f2, double tolerance) { return f1 - f2 < tolerance && f2 - f1 < tolerance; }

#ifndef FLT_MAX
/** The maximum value that can be expressed using a 32-bit floating point value.
	This macro is pre-defined in the Windows API but is missing on MacOSX and other platforms. */
#define FLT_MAX __FLT_MAX__
#endif

#endif // _FM_FLOAT_H_

