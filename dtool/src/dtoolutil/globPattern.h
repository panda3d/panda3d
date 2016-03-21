/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globPattern.h
 * @author drose
 * @date 2000-05-30
 */

#ifndef GLOBPATTERN_H
#define GLOBPATTERN_H

#include "dtoolbase.h"
#include "filename.h"
#include "vector_string.h"

/**
 * This class can be used to test for string matches against standard Unix-
 * shell filename globbing conventions.  It serves as a portable standin for
 * the Posix fnmatch() call.
 *
 * A GlobPattern is given a pattern string, which can contain operators like
 * *, ?, and [].  Then it can be tested against any number of candidate
 * strings; for each candidate, it will indicate whether the string matches
 * the pattern or not.  It can be used, for example, to scan a directory for
 * all files matching a particular pattern.
 */
class EXPCL_DTOOL GlobPattern {
PUBLISHED:
  INLINE GlobPattern(const string &pattern = string());
  INLINE GlobPattern(const GlobPattern &copy);
  INLINE void operator = (const GlobPattern &copy);

  INLINE bool operator == (const GlobPattern &other) const;
  INLINE bool operator != (const GlobPattern &other) const;
  INLINE bool operator < (const GlobPattern &other) const;

  INLINE void set_pattern(const string &pattern);
  INLINE const string &get_pattern() const;
  MAKE_PROPERTY(pattern, get_pattern, set_pattern);

  INLINE void set_case_sensitive(bool case_sensitive);
  INLINE bool get_case_sensitive() const;
  MAKE_PROPERTY(case_sensitive, get_case_sensitive, set_case_sensitive);

  INLINE void set_nomatch_chars(const string &nomatch_chars);
  INLINE const string &get_nomatch_chars() const;
  MAKE_PROPERTY(nomatch_chars, get_nomatch_chars, set_nomatch_chars);

  INLINE bool matches(const string &candidate) const;

  INLINE void output(ostream &out) const;

  bool has_glob_characters() const;
  string get_const_prefix() const;
  int match_files(vector_string &results, const Filename &cwd = Filename()) const;
#ifdef HAVE_PYTHON
  EXTENSION(PyObject *match_files(const Filename &cwd = Filename()) const);
#endif

private:
  bool matches_substr(string::const_iterator pi,
                      string::const_iterator pend,
                      string::const_iterator ci,
                      string::const_iterator cend) const;

  bool matches_set(string::const_iterator &pi,
                   string::const_iterator pend,
                   char ch) const;

  int r_match_files(const Filename &prefix, const string &suffix,
                    vector_string &results, const Filename &cwd);

  string _pattern;
  bool _case_sensitive;
  string _nomatch_chars;
};

INLINE ostream &operator << (ostream &out, const GlobPattern &glob) {
  glob.output(out);
  return out;
}


#include "globPattern.I"

#endif
