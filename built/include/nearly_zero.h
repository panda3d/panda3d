/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nearly_zero.h
 * @author drose
 * @date 2000-03-08
 */

#ifndef NEARLY_ZERO_H
#define NEARLY_ZERO_H

#include "dtoolbase.h"

// The following two functions are defined just to make the NEARLY_ZERO()
// macro work.  They each return a suitable nearly-zero value for their
// corresponding numeric type.

// Note that declaring these small numeric values first as a static const
// identifier, and then returning the value of that identifier, seems to lead
// to compilation errors (at least in VC7) in which sometimes
// IS_THRESHOLD_COMPEQ(a, a, get_nearly_zero_value(a)) != 0.
constexpr double
get_nearly_zero_value(double) {
  return 1.0e-12;
}

constexpr float
get_nearly_zero_value(float) {
  return 1.0e-6f;
}

constexpr int
get_nearly_zero_value(int) {
  // This is a bit silly, but we should nevertheless define it in case it is
  // called for an integer type.
  return 0;
}


// IS_THRESHOLD_ZERO(value, threshold) returns true if the value is within
// threshold of zero.
#define IS_THRESHOLD_ZERO(value, threshold) \
  ((value) < (threshold) && (value) > -(threshold))

// IS_THRESHOLD_EQUAL(value1, value2, threshold) returns true if the two
// values are within threshold of each other.
#define IS_THRESHOLD_EQUAL(value1, value2, threshold) \
  (IS_THRESHOLD_ZERO((value1) - (value2), threshold))

// IS_THRESHOLD_COMPEQ(value1, value2, threshold) returns true if the two
// values are equal within threshold tolerance.  Unlike IS_THRESHOLD_EQUAL,
// the transitive principle is guaranteed: IS_THRESHOLD_COMPEQ(a, b, t) &&
// IS_THRESHOLD_COMPEQ(b, c, t) implies IS_THRESHOLD_COMPEQ(a, c, t).
#define IS_THRESHOLD_COMPEQ(value1, value2, threshold) \
  (cfloor(value1 / threshold + 0.5f) == cfloor(value2 / threshold + 0.5f))

// NEARLY_ZERO(float) returns a number that is considered to be so close to
// zero as not to matter for a float.  NEARLY_ZERO(double) returns a similar,
// smaller number for a double.
#define NEARLY_ZERO(FLOATTYPE) (get_nearly_zero_value((FLOATTYPE)0))

// IS_NEARLY_ZERO(value) returns true if the value is very close to zero.
#define IS_NEARLY_ZERO(value) \
  (IS_THRESHOLD_ZERO(value, get_nearly_zero_value(value)))

// IS_NEARLY_EQUAL(value1, value2) returns true if the two values are very
// close to each other.
#define IS_NEARLY_EQUAL(value1, value2) \
   (IS_THRESHOLD_EQUAL(value1, value2, get_nearly_zero_value(value1)))


// MAYBE_ZERO(value) returns 0 if the value is nearly zero, and the value
// itself otherwise.
#define MAYBE_ZERO(value) \
  (IS_NEARLY_ZERO(value) ? 0 : (value))


#endif
