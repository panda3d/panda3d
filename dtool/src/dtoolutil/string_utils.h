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
#include <string_view>
#include "vector_string.h"
#include "pdtoa.h"

// Case-insensitive string comparison, from Stroustrup's C++ third edition.
// Works like strcmp().
EXPCL_DTOOL_DTOOLUTIL int cmp_nocase(std::string_view s, std::string_view s2);

// Similar, except it also accepts hyphen and underscore as equivalent.
EXPCL_DTOOL_DTOOLUTIL int cmp_nocase_uh(std::string_view s, std::string_view s2);

// Returns the string converted to lowercase.
EXPCL_DTOOL_DTOOLUTIL std::string downcase(std::string_view s);

// Returns the string converted to uppercase.
EXPCL_DTOOL_DTOOLUTIL std::string upcase(std::string_view s);

// Separates the string into words according to whitespace.
EXPCL_DTOOL_DTOOLUTIL int extract_words(std::string_view str, vector_string &words);
EXPCL_DTOOL_DTOOLUTIL int extract_words(std::wstring_view str, pvector<std::wstring> &words);

// Separates the string into words according to the indicated delimiters.
EXPCL_DTOOL_DTOOLUTIL void tokenize(std::string_view str, vector_string &words,
                          std::string_view delimiters,
                          bool discard_repeated_delimiters = false);
EXPCL_DTOOL_DTOOLUTIL void tokenize(std::wstring_view str, pvector<std::wstring> &words,
                          std::wstring_view delimiters,
                          bool discard_repeated_delimiters = false);

// Trims leading andor trailing whitespace from the string.
EXPCL_DTOOL_DTOOLUTIL std::string trim_left(std::string_view str);
EXPCL_DTOOL_DTOOLUTIL std::wstring trim_left(std::wstring_view str);
EXPCL_DTOOL_DTOOLUTIL std::string trim_right(std::string_view str);
EXPCL_DTOOL_DTOOLUTIL std::wstring trim_right(std::wstring_view str);
EXPCL_DTOOL_DTOOLUTIL std::string trim(std::string_view str);
EXPCL_DTOOL_DTOOLUTIL std::wstring trim(std::wstring_view str);

// Functions to parse numeric values out of a string.
EXPCL_DTOOL_DTOOLUTIL int string_to_int(const std::string &str, std::string &tail);
EXPCL_DTOOL_DTOOLUTIL bool string_to_int(const std::string &str, int &result);
EXPCL_DTOOL_DTOOLUTIL double string_to_double(const std::string &str, std::string &tail);
EXPCL_DTOOL_DTOOLUTIL bool string_to_double(const std::string &str, double &result);
EXPCL_DTOOL_DTOOLUTIL bool string_to_float(const std::string &str, float &result);
EXPCL_DTOOL_DTOOLUTIL bool string_to_stdfloat(const std::string &str, PN_stdfloat &result);

// Convenience function to make a string from anything that has an ostream
// operator.
template<class Thing>
INLINE std::string format_string(const Thing &thing);

// Fast specializations for some primitive types.
INLINE std::string format_string(std::string_view value);
INLINE std::string format_string(bool value);
INLINE std::string format_string(float value);
INLINE std::string format_string(double value);
INLINE std::string format_string(unsigned int value);
INLINE std::string format_string(int value);
INLINE std::string format_string(int64_t value);

#include "string_utils.I"

#endif
