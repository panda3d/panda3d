// Filename: globPattern.h
// Created by:  drose (30May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GLOBPATTERN_H
#define GLOBPATTERN_H

#include "dtoolbase.h"
#include "filename.h"
#include "vector_string.h"

////////////////////////////////////////////////////////////////////
//       Class : GlobPattern
// Description : This class can be used to test for string matches
//               against standard Unix-shell filename globbing
//               conventions.  It serves as a portable standin for the
//               Posix fnmatch() call.
//
//               A GlobPattern is given a pattern string, which can
//               contain operators like *, ?, and [].  Then it can be
//               tested against any number of candidate strings; for
//               each candidate, it will indicate whether the string
//               matches the pattern or not.  It can be used, for
//               example, to scan a directory for all files matching a
//               particular pattern.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL GlobPattern {
public:
  INLINE GlobPattern(const string &pattern = string());
  INLINE GlobPattern(const GlobPattern &copy);
  INLINE void operator = (const GlobPattern &copy);

  INLINE void set_pattern(const string &pattern);
  INLINE const string &get_pattern() const;

  INLINE void set_case_sensitive(bool case_sensitive);
  INLINE bool get_case_sensitive() const;

  INLINE bool matches(const string &candidate) const;

  INLINE void output(ostream &out) const;

  bool has_glob_characters() const;
  int match_files(vector_string &results, const Filename &cwd = Filename());

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
};

INLINE ostream &operator << (ostream &out, const GlobPattern &glob) {
  glob.output(out);
  return out;
}


#include "globPattern.I"

#endif
