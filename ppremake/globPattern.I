// Filename: globPattern.I
// Created by:  drose (30May00)
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


////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GlobPattern::
GlobPattern(const string &pattern) : _pattern(pattern) {
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GlobPattern::
GlobPattern(const GlobPattern &copy) : _pattern(copy._pattern) {
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GlobPattern::
operator = (const GlobPattern &copy) {
  _pattern = copy._pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::set_pattern
//       Access: Public
//  Description: Changes the pattern string that the GlobPattern
//               object matches.
////////////////////////////////////////////////////////////////////
INLINE void GlobPattern::
set_pattern(const string &pattern) {
  _pattern = pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::get_pattern
//       Access: Public
//  Description: Returns the pattern string that the GlobPattern
//               object matches.
////////////////////////////////////////////////////////////////////
INLINE const string &GlobPattern::
get_pattern() const {
  return _pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::matches
//       Access: Public
//  Description: Returns true if the candidate string matches the
//               pattern, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GlobPattern::
matches(const string &candidate) const {
  return matches_substr(_pattern.begin(), _pattern.end(),
                        candidate.begin(), candidate.end());
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GlobPattern::
output(ostream &out) const {
  out << _pattern;
}
