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

#include "pandabase.h"

#include <string>
#include "vector_string.h"

// Case-insensitive string comparison, from Stroustrup's C++ third edition.
// Works like strcmp().
EXPCL_PANDA_PUTIL int cmp_nocase(const string &s, const string &s2);

// Similar, except it also accepts hyphen and underscore as equivalent.
EXPCL_PANDA_PUTIL int cmp_nocase_uh(const string &s, const string &s2);

// Returns the string converted to lowercase.
EXPCL_PANDA_PUTIL string downcase(const string &s);

// Returns the string converted to uppercase.
EXPCL_PANDA_PUTIL string upcase(const string &s);

// Separates the string into words according to whitespace.
EXPCL_PANDA_PUTIL int extract_words(const string &str, vector_string &words);
EXPCL_PANDA_PUTIL int extract_words(const wstring &str, pvector<wstring> &words);

// Separates the string into words according to the indicated delimiters.
EXPCL_PANDA_PUTIL void tokenize(const string &str, vector_string &words,
                          const string &delimiters,
                          bool discard_repeated_delimiters = false);
EXPCL_PANDA_PUTIL void tokenize(const wstring &str, pvector<wstring> &words,
                          const wstring &delimiters,
                          bool discard_repeated_delimiters = false);

// Trims leading and/or trailing whitespace from the string.
EXPCL_PANDA_PUTIL string trim_left(const string &str);
EXPCL_PANDA_PUTIL wstring trim_left(const wstring &str);
EXPCL_PANDA_PUTIL string trim_right(const string &str);
EXPCL_PANDA_PUTIL wstring trim_right(const wstring &str);
EXPCL_PANDA_PUTIL string trim(const string &str);
EXPCL_PANDA_PUTIL wstring trim(const wstring &str);

// Functions to parse numeric values out of a string.
EXPCL_PANDA_PUTIL int string_to_int(const string &str, string &tail);
EXPCL_PANDA_PUTIL bool string_to_int(const string &str, int &result);
EXPCL_PANDA_PUTIL double string_to_double(const string &str, string &tail);
EXPCL_PANDA_PUTIL bool string_to_double(const string &str, double &result);
EXPCL_PANDA_PUTIL bool string_to_float(const string &str, float &result);
EXPCL_PANDA_PUTIL bool string_to_stdfloat(const string &str, PN_stdfloat &result);

// Convenience function to make a string from anything that has an
// ostream operator.
template<class Thing>
INLINE string format_string(const Thing &thing);

#include "string_utils.I"

#endif

