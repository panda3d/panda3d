// Filename: nearly_zero.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NEARLY_ZERO_H
#define NEARLY_ZERO_H


// The following two functions are defined just to make the
// NEARLY_ZERO() macro work.  They each return a suitable nearly-zero
// value for their corresponding numeric type.
INLINE double
get_nearly_zero_value(double) {
  return 1.0e-12;
}

INLINE float
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


// NEARLY_ZERO(float) returns a number that is considered to be so
// close to zero as not to matter for a float.  NEARLY_ZERO(double)
// returns a similar, smaller number for a double.
#define NEARLY_ZERO(FLOATTYPE1) (get_nearly_zero_value((FLOATTYPE1)0))

// IS_NEARLY_ZERO(value) returns true if the value is very close to
// zero.
#define IS_NEARLY_ZERO(value) \
  (IS_THRESHOLD_ZERO(value, get_nearly_zero_value(value)))

// IS_NEARLY_EQUAL(value1, value2) returns true if the two values are
// very close to each other.
#define IS_NEARLY_EQUAL(value1, value2) \
   IS_NEARLY_ZERO((value1) - (value2))


// MAYBE_ZERO(value) returns 0 if the value is nearly zero, and the
// value itself otherwise.
#define MAYBE_ZERO(value) \
  (IS_NEARLY_ZERO(value) ? 0.0 : value)


#endif

