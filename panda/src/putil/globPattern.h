// Filename: globPattern.h
// Created by:  drose (30May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GLOBPATTERN_H
#define GLOBPATTERN_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
// 	 Class : GlobPattern
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
class EXPCL_PANDA GlobPattern {
public:
  INLINE GlobPattern(const string &pattern = string());

  INLINE void set_pattern(const string &pattern);
  INLINE const string &get_pattern() const;

  INLINE bool matches(const string &candidate) const;

private:
  bool matches_substr(string::const_iterator pi,
		      string::const_iterator pend,
		      string::const_iterator ci,
		      string::const_iterator cend) const;

  bool matches_set(string::const_iterator &pi, 
		   string::const_iterator pend,
		   char ch) const;

  string _pattern;
};

#include "globPattern.I"

#endif
