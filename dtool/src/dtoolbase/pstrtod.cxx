/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pstrtod.cxx
 * @author drose
 * @date 2009-06-13
 */

#include "pstrtod.h"

#include <ctype.h>
#include <math.h>
#include <limits>

#ifdef _WIN32
#define strncasecmp _strnicmp
#endif

/**
 * This function re-implements strtod, to avoid the problems that occur when
 * the LC_NUMERIC locale gets set to anything other than "C".  Regardless of
 * the user's locale, we need to be able to parse floating-point numbers
 * internally understanding a "." as the decimal point.
 */
double
pstrtod(const char *nptr, char **endptr) {
  // First, skip whitespace.
  const char *p = nptr;
  while (isspace(*p)) {
    ++p;
  }

  // Skip an optional leading sign.
  char sign = '+';
  if (*p == '+' || *p == '-') {
    sign = *p;
    ++p;
  }

  double value = 0.0;

  if (isalpha(*p)) {
    // Windows' implementation of strtod doesn't support "inf" or "nan", so
    // check for those here.
    if (strncasecmp(p, "inf", 3) == 0) {
      p += 3;
      if (strncasecmp(p, "inity", 5) == 0) {
        p += 5;
      }
      value = std::numeric_limits<double>::infinity();

    } else if (strncasecmp(p, "nan", 3) == 0) {
      p += 3;

      if (*p == 's' || *p == 'S') {
        value = std::numeric_limits<double>::signaling_NaN();
        ++p;
      } else {
        if (*p == 'q' || *p == 'Q') {
          ++p;
        }
        value = std::numeric_limits<double>::quiet_NaN();
      }

      // It is optionally possible to include a character sequence between
      // parentheses after "nan", to be passed to the new nan() function.
      // Since it isn't supported universally, we will only accept a pair of
      // empty parentheses.
      if (strncmp(p, "()", 2) == 0) {
        p += 2;
      }

    } else {
      // Pass it up to the system implementation of strtod; perhaps it knows
      // how to deal with this string.
      return strtod(nptr, endptr);
    }

  } else {
    // Start reading decimal digits to the left of the decimal point.
    bool found_digits = false;
    while (isdigit(*p)) {
      value = (value * 10.0) + (*p - '0');
      found_digits = true;
      ++p;
    }

    if (*p == '.') {
      ++p;
      // Read decimal digits to the right of the decimal point.
      double multiplicand = 0.1;
      while (isdigit(*p)) {
        value += (*p - '0') * multiplicand;
        ++p;
        found_digits = true;
        multiplicand *= 0.1;
      }
    }

    if (!found_digits) {
      // Not a valid float.
      if (endptr != nullptr) {
        *endptr = (char *)nptr;
      }
      return 0.0;
    }

    if (tolower(*p) == 'e') {
      // There's an exponent.
      ++p;

      char esign = '+';
      if (*p == '+' || *p == '-') {
        esign = *p;
        ++p;
      }

      // Start reading decimal digits to the left of the decimal point.
      double evalue = 0.0;
      while (isdigit(*p)) {
        evalue = (evalue * 10.0) + (*p - '0');
        ++p;
      }

      if (esign == '-') {
        value /= pow(10.0, evalue);
      } else {
        value *= pow(10.0, evalue);
      }
    }
  }

  if (sign == '-') {
    value = -value;
  }

  if (endptr != nullptr) {
    *endptr = (char *)p;
  }
  return value;
}


/**
 * This function re-implements atof, to avoid the problems that occur when the
 * LC_NUMERIC locale gets set to anything other than "C".  Regardless of the
 * user's locale, we need to be able to parse floating-point numbers
 * internally understanding a "." as the decimal point.
 */
double
patof(const char *str) {
  return pstrtod(str, nullptr);
}
