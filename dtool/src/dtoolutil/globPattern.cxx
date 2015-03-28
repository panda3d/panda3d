// Filename: globPattern.cxx
// Created by:  drose (30May00)
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

#include "globPattern.h"
#include <ctype.h>

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::has_glob_characters
//       Access: Published
//  Description: Returns true if the pattern includes any special
//               globbing characters, or false if it is just a literal
//               string.
////////////////////////////////////////////////////////////////////
bool GlobPattern::
has_glob_characters() const {
  string::const_iterator pi;
  pi = _pattern.begin();
  while (pi != _pattern.end()) {
    switch (*pi) {
    case '*':
    case '?':
    case '[':
      return true;

    case '\\':
      ++pi;
      if (pi == _pattern.end()) {
        return false;
      }
    }
    ++pi;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::get_const_prefix
//       Access: Published
//  Description: Returns the initial part of the pattern before the
//               first glob character.  Since many glob patterns begin
//               with a sequence of static characters and end with one
//               or more glob characters, this can be used to
//               optimized searches through sorted indices.
////////////////////////////////////////////////////////////////////
string GlobPattern::
get_const_prefix() const {
  string prefix;

  size_t p = 0;  // current point
  size_t q = 0;  // starting point
  while (p < _pattern.size()) {
    switch (_pattern[p]) {
    case '*':
    case '?':
    case '[':
      return prefix + _pattern.substr(q, p - q);

    case '\\':
      // Skip over the backslash.
      prefix += _pattern.substr(q, p - q);
      ++p;
      q = p;
    }
    ++p;
  }
  return prefix += _pattern.substr(q, p - q);
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::match_files
//       Access: Published
//  Description: Treats the GlobPattern as a filename pattern, and
//               returns a list of any actual files that match the
//               pattern.  This is the behavior of the standard Posix
//               glob() function.  Any part of the filename may
//               contain glob characters, including intermediate
//               directory names.
//
//               If cwd is specified, it is the directory that
//               relative filenames are taken to be relative to;
//               otherwise, the actual current working directory is
//               assumed.
//
//               The return value is the number of files matched,
//               which are added to the results vector.
////////////////////////////////////////////////////////////////////
int GlobPattern::
match_files(vector_string &results, const Filename &cwd) const {
  string prefix, pattern, suffix;

  string source = _pattern;
  if (!source.empty() && source[0] == '/') {
    // If the first character is a slash, that becomes the prefix.
    prefix = "/";
    source = source.substr(1);
  }

  size_t slash = source.find('/');
  if (slash == string::npos) {
    pattern = source;
  } else {
    pattern = source.substr(0, slash);
    suffix = source.substr(slash + 1);
  }

  GlobPattern glob(pattern);
  glob.set_case_sensitive(_case_sensitive);
  return glob.r_match_files(prefix, suffix, results, cwd);
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::r_match_files
//       Access: Private
//  Description: The recursive implementation of match_files().
////////////////////////////////////////////////////////////////////
int GlobPattern::
r_match_files(const Filename &prefix, const string &suffix,
              vector_string &results, const Filename &cwd) {
  string next_pattern, next_suffix;

  size_t slash = suffix.find('/');
  if (slash == string::npos) {
    next_pattern = suffix;
  } else {
    next_pattern = suffix.substr(0, slash);
    next_suffix = suffix.substr(slash + 1);
  }

  Filename parent_dir;
  if (prefix.is_local() && !cwd.empty()) {
    parent_dir = Filename(cwd, prefix);
  } else {
    parent_dir = prefix;
  }

  GlobPattern next_glob(next_pattern);
  next_glob.set_case_sensitive(_case_sensitive);

  if (!has_glob_characters()) {
    // If there are no special characters in the pattern, it's a
    // literal match.
    if (suffix.empty()) {
      // Time to stop.
      Filename single_filename(parent_dir, _pattern);
      if (single_filename.exists()) {
        results.push_back(Filename(prefix, _pattern));
        return 1;
      }
      return 0;
    }

    return next_glob.r_match_files(Filename(prefix, _pattern),
                                   next_suffix, results, cwd);

  }

  // If there *are* special glob characters, we must attempt to
  // match the pattern against the files in this directory.

  vector_string dir_files;
  if (!parent_dir.scan_directory(dir_files)) {
    // Not a directory, or unable to read directory; stop here.
    return 0;
  }

  // Now go through each file in the directory looking for one that
  // matches the pattern.
  int num_matched = 0;

  vector_string::const_iterator fi;
  for (fi = dir_files.begin(); fi != dir_files.end(); ++fi) {
    const string &local_file = (*fi);
    if (_pattern[0] == '.' || (local_file.empty() || local_file[0] != '.')) {
      if (matches(local_file)) {
        // We have a match; continue.
        if (suffix.empty()) {
          results.push_back(Filename(prefix, local_file));
          num_matched++;
        } else {
          num_matched += next_glob.r_match_files(Filename(prefix, local_file),
                                                 next_suffix, results, cwd);
        }
      }
    }
  }

  return num_matched;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::matches_substr
//       Access: Private
//  Description: The recursive implementation of matches().  This
//               returns true if the pattern substring [pi, pend)
//               matches the candidate substring [ci, cend), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GlobPattern::
matches_substr(string::const_iterator pi, string::const_iterator pend,
               string::const_iterator ci, string::const_iterator cend) const {
  // If we run out of pattern or candidate string, it's a match only
  // if they both ran out at the same time.
  if (pi == pend || ci == cend) {
    // A special exception: we allow ci to reach the end before pi,
    // only if pi is one character before the end and that last
    // character is '*'.
    if ((ci == cend) && (std::distance(pi, pend) == 1) && (*pi) == '*') {
      return true;
    }
    return (pi == pend && ci == cend);
  }

  switch (*pi) {

  case '*':
    // A '*' in the pattern string means to match any sequence of zero
    // or more characters in the candidate string.  This means we have
    // to recurse twice: either consume one character of the candidate
    // string and continue to try matching the *, or stop trying to
    // match the * here.
    if (_nomatch_chars.find(*ci) == string::npos) {
      return
        matches_substr(pi, pend, ci + 1, cend) ||
        matches_substr(pi + 1, pend, ci, cend);
    } else {
      // On the other hand, if this is one of the nomatch chars, we
      // can only stop here.
      return matches_substr(pi + 1, pend, ci, cend);
    }

  case '?':
    // A '?' in the pattern string means to match exactly one
    // character in the candidate string.  That's easy.
    return matches_substr(pi + 1, pend, ci + 1, cend);

  case '[':
    // An open square bracket begins a set.
    ++pi;
    if ((*pi) == '!') {
      ++pi;
      if (matches_set(pi, pend, *ci)) {
        return false;
      }
    } else {
      if (!matches_set(pi, pend, *ci)) {
        return false;
      }
    }
    if (pi == pend) {
      // Oops, there wasn't a closing square bracket.
      return false;
    }
    return matches_substr(pi + 1, pend, ci + 1, cend);

  case '\\':
    // A backslash escapes the next special character.
    ++pi;
    if (pi == pend) {
      return false;
    }
    // fall through.

  default:
    // Anything else means to match exactly that.
    if (_case_sensitive) {
      if ((*pi) != (*ci)) {
        return false;
      }
    } else {
      if (tolower(*pi) != tolower(*ci)) {
        return false;
      }
    }
    return matches_substr(pi + 1, pend, ci + 1, cend);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::matches_set
//       Access: Private
//  Description: Called when an unescaped open square bracked is
//               scanned, this is called with pi positioned after the
//               opening square bracket, scans the set sequence,
//               leaving pi positioned on the closing square bracket,
//               and returns true if the indicated character matches
//               the set of characters indicated, false otherwise.
////////////////////////////////////////////////////////////////////
bool GlobPattern::
matches_set(string::const_iterator &pi, string::const_iterator pend,
            char ch) const {
  bool matched = false;

  while (pi != pend && (*pi) != ']') {
    if ((*pi) == '\\') {
      // Backslash escapes the next character.
      ++pi;
      if (pi == pend) {
        return false;
      }
    }

    if (ch == (*pi)) {
      matched = true;
    }

    // Maybe it's an a-z style range?
    char start = (*pi);
    ++pi;
    if (pi != pend && (*pi) == '-') {
      ++pi;
      if (pi != pend && (*pi) != ']') {
        // Yes, we have a range: start-end.

        if ((*pi) == '\\') {
          // Backslash escapes.
          ++pi;
          if (pi == pend) {
            return false;
          }
        }

        char end = (*pi);
        ++pi;

        if ((ch >= start && ch <= end) || 
            (!_case_sensitive && 
             ((tolower(ch) >= start && tolower(ch) <= end) ||
              (toupper(ch) >= start && toupper(ch) <= end)))) {
          matched = true;
        }
      } else {
        // This was a - at the end of the string.
        if (ch == '-') {
          matched = true;
        }
      }
    }
  }

  return matched;
}



