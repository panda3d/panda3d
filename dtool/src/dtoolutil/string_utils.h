// Filename: string_utils.h
// Created by:  drose (18Jan99)
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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "dtoolbase.h"

#include <string>
#include "vector_string.h"
#include "pdtoa.h"

// Case-insensitive string comparison, from Stroustrup's C++ third edition.
// Works like strcmp().
EXPCL_DTOOL int cmp_nocase(const string &s, const string &s2);

// Similar, except it also accepts hyphen and underscore as equivalent.
EXPCL_DTOOL int cmp_nocase_uh(const string &s, const string &s2);

// Returns the string converted to lowercase.
EXPCL_DTOOL string downcase(const string &s);

// Returns the string converted to uppercase.
EXPCL_DTOOL string upcase(const string &s);

// Separates the string into words according to whitespace.
EXPCL_DTOOL int extract_words(const string &str, vector_string &words);
EXPCL_DTOOL int extract_words(const wstring &str, pvector<wstring> &words);

// Separates the string into words according to the indicated delimiters.
EXPCL_DTOOL void tokenize(const string &str, vector_string &words,
                          const string &delimiters,
                          bool discard_repeated_delimiters = false);
EXPCL_DTOOL void tokenize(const wstring &str, pvector<wstring> &words,
                          const wstring &delimiters,
                          bool discard_repeated_delimiters = false);

// Trims leading and/or trailing whitespace from the string.
EXPCL_DTOOL string trim_left(const string &str);
EXPCL_DTOOL wstring trim_left(const wstring &str);
EXPCL_DTOOL string trim_right(const string &str);
EXPCL_DTOOL wstring trim_right(const wstring &str);
EXPCL_DTOOL string trim(const string &str);
EXPCL_DTOOL wstring trim(const wstring &str);

// Functions to parse numeric values out of a string.
EXPCL_DTOOL int string_to_int(const string &str, string &tail);
EXPCL_DTOOL bool string_to_int(const string &str, int &result);
EXPCL_DTOOL double string_to_double(const string &str, string &tail);
EXPCL_DTOOL bool string_to_double(const string &str, double &result);
EXPCL_DTOOL bool string_to_float(const string &str, float &result);
EXPCL_DTOOL bool string_to_stdfloat(const string &str, PN_stdfloat &result);

// Convenience function to make a string from anything that has an
// ostream operator.
template<class Thing>
INLINE string format_string(const Thing &thing);

// Fast specializations for some primitive types.
INLINE string format_string(const string &value);
INLINE string format_string(bool value);
INLINE string format_string(float value);
INLINE string format_string(double value);
INLINE string format_string(unsigned int value);
INLINE string format_string(int value);
INLINE string format_string(PN_int64 value);

#include "string_utils.I"

#endif
