// Filename: nearly_zero.h
// Created by:  drose (08Mar00)
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

#ifndef NEARLY_ZERO_H
#define NEARLY_ZERO_H


// The following two functions are defined just to make the
// NEARLY_ZERO() macro work.  They each return a suitable nearly-zero
// value for their corresponding numeric type.
INLINE_LINMATH double
get_nearly_zero_value(double) {
  return 1.0e-12;
}

INLINE_LINMATH float
get_nearly_zero_value(float) {
  return 1.0e-6f;
}


// IS_THRESHOLD_ZERO(value, threshold) returns true if the value is
// within threshold of zero.
#define IS_THRESHOLD_ZERO(value, threshold) \
  ((value) < (threshold) && (value) > -(threshold))

// IS_THRESHOLD_EQUAL(value1, value2, threshold) returns true if the
// two values are within threshold of each other.
#define IS_THRESHOLD_EQUAL(value1, value2, threshold) \
  (IS_THRESHOLD_ZERO((value1) - (value2), threshold))

// IS_THRESHOLD_COMPEQ(value1, value2, threshold) returns true if
// the two values are equal within threshold tolerance.  Unlike
// IS_THRESHOLD_EQUAL, the transitive principle is guaranteed:
// IS_THRESHOLD_COMPEQ(a, b, t) && IS_THRESHOLD_COMPEQ(b, c, t)
// implies IS_THRESHOLD_COMPEQ(a, c, t).
#define IS_THRESHOLD_COMPEQ(value1, value2, threshold) \
  (cfloor(value1 / threshold + 0.5f) == cfloor(value2 / threshold + 0.5f))

// NEARLY_ZERO(float) returns a number that is considered to be so
// close to zero as not to matter for a float.  NEARLY_ZERO(double)
// returns a similar, smaller number for a double.
#define NEARLY_ZERO(FLOATTYPE) (get_nearly_zero_value((FLOATTYPE)0))

// IS_NEARLY_ZERO(value) returns true if the value is very close to
// zero.
#define IS_NEARLY_ZERO(value) \
  (IS_THRESHOLD_ZERO(value, get_nearly_zero_value(value)))

// IS_NEARLY_EQUAL(value1, value2) returns true if the two values are
// very close to each other.
#define IS_NEARLY_EQUAL(value1, value2) \
   (IS_THRESHOLD_EQUAL(value1, value2, get_nearly_zero_value(value1)))


// MAYBE_ZERO(value) returns 0 if the value is nearly zero, and the
// value itself otherwise.
#define MAYBE_ZERO(value) \
  (IS_NEARLY_ZERO(value) ? 0.0 : (value))


#endif

