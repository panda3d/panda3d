// Filename: string_utils.h
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <pandabase.h>

#include <string>

// Case-insensitive string comparison, from Stroustrup's C++ third edition.
// Works like strcmp().
EXPCL_PANDA int cmp_nocase(const string &s, const string &s2);

// Similar, except it also accepts hyphen and underscore as equivalent.
EXPCL_PANDA int cmp_nocase_uh(const string &s, const string &s2);

// Returns the string converted to lowercase.
EXPCL_PANDA string downcase(const string &s);

// Convenience function to make a string from anything that has an
// ostream operator.
template<class Thing>
INLINE string format_string(const Thing &thing);

#include "string_utils.I"

#endif

