/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file string_utils.h
 * @author drose
 * @date 1999-01-18
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "dtoolbase.h"

#include <string>
#include "vector_string.h"
#include "pdtoa.h"

// Case-insensitive string comparison, from Stroustrup's C++ third edition.
// Works like strcmp().
EXPCL_DTOOL_DTOOLUTIL int cmp_nocase(const string &s, const string &s2);

// Similar, except it also accepts hyphen and underscore as equivalent.
EXPCL_DTOOL_DTOOLUTIL int cmp_nocase_uh(const string &s, const string &s2);

// Returns the string converted to lowercase.
EXPCL_DTOOL_DTOOLUTIL string downcase(const string &s);

// Returns the string converted to uppercase.
EXPCL_DTOOL_DTOOLUTIL string upcase(const string &s);

// Separates the string into words according to whitespace.
EXPCL_DTOOL_DTOOLUTIL int extract_words(const string &str, vector_string &words);
EXPCL_DTOOL_DTOOLUTIL int extract_words(const wstring &str, pvector<wstring> &words);

// Separates the string into words according to the indicated delimiters.
EXPCL_DTOOL_DTOOLUTIL void tokenize(const string &str, vector_string &words,
                          const string &delimiters,
                          bool discard_repeated_delimiters = false);
EXPCL_DTOOL_DTOOLUTIL void tokenize(const wstring &str, pvector<wstring> &words,
                          const wstring &delimiters,
                          bool discard_repeated_delimiters = false);

// Trims leading andor trailing whitespace from the string.
EXPCL_DTOOL_DTOOLUTIL string trim_left(const string &str);
EXPCL_DTOOL_DTOOLUTIL wstring trim_left(const wstring &str);
EXPCL_DTOOL_DTOOLUTIL string trim_right(const string &str);
EXPCL_DTOOL_DTOOLUTIL wstring trim_right(const wstring &str);
EXPCL_DTOOL_DTOOLUTIL string trim(const string &str);
EXPCL_DTOOL_DTOOLUTIL wstring trim(const wstring &str);

// Functions to parse numeric values out of a string.
EXPCL_DTOOL_DTOOLUTIL int string_to_int(const string &str, string &tail);
EXPCL_DTOOL_DTOOLUTIL bool string_to_int(const string &str, int &result);
EXPCL_DTOOL_DTOOLUTIL double string_to_double(const string &str, string &tail);
EXPCL_DTOOL_DTOOLUTIL bool string_to_double(const string &str, double &result);
EXPCL_DTOOL_DTOOLUTIL bool string_to_float(const string &str, float &result);
EXPCL_DTOOL_DTOOLUTIL bool string_to_stdfloat(const string &str, PN_stdfloat &result);

// Convenience function to make a string from anything that has an ostream
// operator.
template<class Thing>
INLINE string format_string(const Thing &thing);

// Fast specializations for some primitive types.
INLINE string format_string(const string &value);
INLINE string format_string(bool value);
INLINE string format_string(float value);
INLINE string format_string(double value);
INLINE string format_string(unsigned int value);
INLINE string format_string(int value);
INLINE string format_string(int64_t value);

#include "string_utils.I"

#endif
