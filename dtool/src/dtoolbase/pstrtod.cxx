// Filename: pstrtod.cxx
// Created by:  drose (13Jun09)
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

#include "pstrtod.h"

#include <ctype.h>
#include <math.h>


////////////////////////////////////////////////////////////////////
//     Function: pstrtod
//  Description: This function re-implements strtod, to avoid the
//               problems that occur when the LC_NUMERIC locale gets
//               set to anything other than "C".  Regardless of the
//               user's locale, we need to be able to parse
//               floating-point numbers internally understanding a "."
//               as the decimal point.
////////////////////////////////////////////////////////////////////
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
    // For special cases like "inf" and "nan", pass these up to the
    // system implementation of strtod.
    return strtod(nptr, endptr);
  }

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
    if (endptr != NULL) {
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
      value /= pow(evalue, 10.0);
    } else {
      value *= pow(evalue, 10.0);
    }
  }        

  if (sign == '-') {
    value = -value;
  }
  
  if (endptr != NULL) {
    *endptr = (char *)p;
  }
  return value;
}


////////////////////////////////////////////////////////////////////
//     Function: patof
//  Description: This function re-implements atof, to avoid the
//               problems that occur when the LC_NUMERIC locale gets
//               set to anything other than "C".  Regardless of the
//               user's locale, we need to be able to parse
//               floating-point numbers internally understanding a "."
//               as the decimal point.
////////////////////////////////////////////////////////////////////
double
patof(const char *str) {
  return pstrtod(str, (char **)NULL);
}
