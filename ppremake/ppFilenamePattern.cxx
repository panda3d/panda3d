// Filename: ppFilenamePattern.cxx
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "ppFilenamePattern.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPFilenamePattern::
PPFilenamePattern(const string &pattern) {
  size_t pct = pattern.find(PATTERN_WILDCARD);
  _has_wildcard = (pct != string::npos);
  
  if (_has_wildcard) {
    _prefix = pattern.substr(0, pct);
    _suffix = pattern.substr(pct + 1);
  } else {
    _prefix = pattern;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPFilenamePattern::
PPFilenamePattern(const PPFilenamePattern &copy) :
  _has_wildcard(copy._has_wildcard),
  _prefix(copy._prefix),
  _suffix(copy._suffix)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PPFilenamePattern::
operator = (const PPFilenamePattern &copy) {
  _has_wildcard = copy._has_wildcard;
  _prefix = copy._prefix;
  _suffix = copy._suffix;
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::has_wildcard
//       Access: Public
//  Description: Returns true if the filename pattern contained a
//               wildcard (and hence represents a pattern and not
//               just a single particular filename), or false if it
//               did not.
////////////////////////////////////////////////////////////////////
bool PPFilenamePattern::
has_wildcard() const {
  return _has_wildcard;
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::get_pattern
//       Access: Public
//  Description: Returns the filename pattern.
////////////////////////////////////////////////////////////////////
string PPFilenamePattern::
get_pattern() const {
  if (_has_wildcard) {
    return _prefix + PATTERN_WILDCARD + _suffix;
  } else {
    return _prefix;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::get_prefix
//       Access: Public
//  Description: Returns the part of the filename pattern before the
//               wildcard.  If the filename did not contain a
//               wildcard, this returns the entire filename.
////////////////////////////////////////////////////////////////////
const string &PPFilenamePattern::
get_prefix() const {
  return _prefix;
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::get_suffix
//       Access: Public
//  Description: Returns the part of the filename pattern after the
//               wildcard.  If the filename did not contain a
//               wildcard, this returns the empty string.
////////////////////////////////////////////////////////////////////
const string &PPFilenamePattern::
get_suffix() const {
  return _suffix;
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::matches_filename
//       Access: Public
//  Description: Returns true if the given filename matches the
//               pattern, false otherwise.
////////////////////////////////////////////////////////////////////
bool PPFilenamePattern::
matches(const string &filename) const {
  if (_has_wildcard) {
    return 
      (filename.length() >= _prefix.length() + _suffix.length()) &&
      (filename.substr(0, _prefix.length()) == _prefix) &&
      (filename.substr(filename.length() - _suffix.length()) == _suffix);

  } else {
    return (filename == _prefix);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::extract_body
//       Access: Public
//  Description: If the filename matches the pattern
//               (e.g. matches_filename() returns true), return the
//               portion of the filename that corresponds to the
//               wildcard in the pattern: the part of the filename
//               between the prefix and the suffix.  If the filename
//               does not match the pattern, or the pattern does not
//               contain a wildcard, returns empty string.
////////////////////////////////////////////////////////////////////
string PPFilenamePattern::
extract_body(const string &filename) const {
  if (_has_wildcard) {
    size_t outside_length = _prefix.length() + _suffix.length();
    if ((filename.length() >= outside_length) &&
        (filename.substr(0, _prefix.length()) == _prefix) &&
        (filename.substr(filename.length() - _suffix.length()) == _suffix)) {
      return filename.substr(_prefix.length(), filename.length() - outside_length);
    }
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PPFilenamePattern::transform_filename
//       Access: Public
//  Description: Transforms a filename by replacing the parts of the
//               filename described in the pattern transform_from with
//               the corresponding parts of the filename described in
//               this pattern.  If the filename does not match
//               transform_from, returns the untransformed filename.
//
//               It is an error to call this unless both this pattern
//               and transform_from include a wildcard.
////////////////////////////////////////////////////////////////////
string PPFilenamePattern::
transform(const string &filename,
          const PPFilenamePattern &transform_from) const {
  assert(transform_from._has_wildcard);

  if (transform_from.matches(filename)) {
    if (!_has_wildcard) {
      return _prefix;
    } else {
      string body = transform_from.extract_body(filename);
      string result = _prefix + body;

      // Now the suffix might contain more % characters.  Replace all
      // of them.
      size_t p = 0;
      size_t pct = _suffix.find(PATTERN_WILDCARD, p);
      while (pct != string::npos) {
        result += _suffix.substr(p, pct - p);
        result += body;
        p = pct + 1;
        pct = _suffix.find(PATTERN_WILDCARD, p);
      }
      result += _suffix.substr(p);

      return result;
    }
  }

  return filename;
}

