/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globPattern.I
 * @author drose
 * @date 2000-05-30
 */

/**
 *
 */
INLINE GlobPattern::
GlobPattern(const std::string &pattern) : _pattern(pattern) {
  _case_sensitive = true;
}

/**
 *
 */
INLINE GlobPattern::
GlobPattern(const GlobPattern &copy) :
  _pattern(copy._pattern),
  _case_sensitive(copy._case_sensitive)
{
}

/**
 *
 */
INLINE void GlobPattern::
operator = (const GlobPattern &copy) {
  _pattern = copy._pattern;
  _case_sensitive = copy._case_sensitive;
}

/**
 *
 */
INLINE bool GlobPattern::
operator == (const GlobPattern &other) const {
  return (_pattern == other._pattern && _case_sensitive == other._case_sensitive);
}

/**
 *
 */
INLINE bool GlobPattern::
operator != (const GlobPattern &other) const {
  return !operator == (other);
}

/**
 *
 */
INLINE bool GlobPattern::
operator < (const GlobPattern &other) const {
  if (_case_sensitive != other._case_sensitive) {
    return (int)_case_sensitive < (int)other._case_sensitive;
  }
  return _pattern < other._pattern;
}

/**
 * Changes the pattern string that the GlobPattern object matches.
 */
INLINE void GlobPattern::
set_pattern(const std::string &pattern) {
  _pattern = pattern;
}

/**
 * Returns the pattern string that the GlobPattern object matches.
 */
INLINE const std::string &GlobPattern::
get_pattern() const {
  return _pattern;
}

/**
 * Sets whether the match is case sensitive (true) or case insensitive
 * (false).  The default is case sensitive.
 */
INLINE void GlobPattern::
set_case_sensitive(bool case_sensitive) {
  _case_sensitive = case_sensitive;
}

/**
 * Returns whether the match is case sensitive (true) or case insensitive
 * (false).  The default is case sensitive.
 */
INLINE bool GlobPattern::
get_case_sensitive() const {
  return _case_sensitive;
}

/**
 * Specifies a set of characters that are not matched by * or ?.
 */
INLINE void GlobPattern::
set_nomatch_chars(const std::string &nomatch_chars) {
  _nomatch_chars = nomatch_chars;
}

/**
 * Returns the set of characters that are not matched by * or ?.
 */
INLINE const std::string &GlobPattern::
get_nomatch_chars() const {
  return _nomatch_chars;
}

/**
 * Returns true if the candidate string matches the pattern, false otherwise.
 */
INLINE bool GlobPattern::
matches(const std::string &candidate) const {
  return matches_substr(_pattern.begin(), _pattern.end(),
                        candidate.begin(), candidate.end());
}

/**
 *
 */
INLINE void GlobPattern::
output(std::ostream &out) const {
  out << _pattern;
}
