// Filename: string_utils.h
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "pandabase.h"

#include <string>
#include <vector_string.h>

// Case-insensitive string comparison, from Stroustrup's C++ third edition.
// Works like strcmp().
EXPCL_PANDA int cmp_nocase(const string &s, const string &s2);

// Similar, except it also accepts hyphen and underscore as equivalent.
EXPCL_PANDA int cmp_nocase_uh(const string &s, const string &s2);

// Returns the string converted to lowercase.
EXPCL_PANDA string downcase(const string &s);

// Returns the string converted to uppercase.
EXPCL_PANDA string upcase(const string &s);

// Separates the string into words according to whitespace.
EXPCL_PANDA int extract_words(const string &str, vector_string &words);

// Separates the string into words according to the indicated delimiters.
EXPCL_PANDA void tokenize(const string &str, vector_string &words,
                          const string &delimiters);

// Trims leading and/or trailing whitespace from the string.
EXPCL_PANDA string trim_left(const string &str);
EXPCL_PANDA string trim_right(const string &str);
EXPCL_PANDA string trim(const string &str);

// Functions to parse numeric values out of a string.
EXPCL_PANDA int string_to_int(const string &str, string &tail);
EXPCL_PANDA bool string_to_int(const string &str, int &result);
EXPCL_PANDA double string_to_double(const string &str, string &tail);
EXPCL_PANDA bool string_to_double(const string &str, double &result);
EXPCL_PANDA bool string_to_float(const string &str, float &result);

// Convenience function to make a string from anything that has an
// ostream operator.
template<class Thing>
INLINE string format_string(const Thing &thing);

#include "string_utils.I"

#endif

