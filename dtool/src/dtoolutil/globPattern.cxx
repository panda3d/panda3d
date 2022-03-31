/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globPattern.cxx
 * @author drose
 * @date 2000-05-30
 */

#include "globPattern.h"
#include "string_utils.h"
#include <ctype.h>

using std::string;

/**
 * Returns true if the pattern includes any special globbing characters, or
 * false if it is just a literal string.
 */
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

/**
 * Returns the initial part of the pattern before the first glob character.
 * Since many glob patterns begin with a sequence of static characters and end
 * with one or more glob characters, this can be used to optimized searches
 * through sorted indices.
 */
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

/**
 * Treats the GlobPattern as a filename pattern, and returns a list of any
 * actual files that match the pattern.  This is the behavior of the standard
 * Posix glob() function.  Any part of the filename may contain glob
 * characters, including intermediate directory names.
 *
 * If cwd is specified, it is the directory that relative filenames are taken
 * to be relative to; otherwise, the actual current working directory is
 * assumed.
 *
 * The return value is the number of files matched, which are added to the
 * results vector.
 */
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
    suffix = source.substr(slash);
  }

  GlobPattern glob(pattern);
  glob.set_case_sensitive(_case_sensitive);
  return glob.r_match_files(prefix, suffix, results, cwd);
}

/**
 * The recursive implementation of match_files().
 */
int GlobPattern::
r_match_files(const Filename &prefix, const string &suffix,
              vector_string &results, const Filename &cwd) {
  string next_pattern, next_suffix;

  size_t slash = suffix.find('/');
  if (slash == string::npos) {
    next_pattern = suffix;
  } else if (slash + 1 == suffix.size()) {
    // If the slash is at the end, we need to keep it, since it indicates that
    // we only want to match directories.
    next_pattern = suffix.substr(0, slash);
    next_suffix = "/";
  } else {
    next_pattern = suffix.substr(0, slash);
    next_suffix = suffix.substr(slash + 1);
  }

  if (_pattern == "**" && next_pattern == "**") {
    // Collapse consecutive globstar patterns.
    return r_match_files(prefix, next_suffix, results, cwd);
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
    // If there are no special characters in the pattern, it's a literal
    // match.
    Filename fn(parent_dir, _pattern);
    if (suffix.empty()) {
      // Time to stop.
      if (fn.exists()) {
        results.push_back(Filename(prefix, _pattern));
        return 1;
      }
      return 0;
    } else if (fn.is_directory()) {
      // If the pattern ends with a slash, match a directory only.
      if (suffix == "/") {
        results.push_back(Filename(prefix, _pattern + "/"));
        return 1;
      } else {
        return next_glob.r_match_files(Filename(prefix, _pattern),
                                       next_suffix, results, cwd);
      }
    }
  }

  // If there *are* special glob characters, we must attempt to match the
  // pattern against the files in this directory.

  vector_string dir_files;
  if (!parent_dir.scan_directory(dir_files)) {
    // Not a directory, or unable to read directory; stop here.
    return 0;
  }

  // Now go through each file in the directory looking for one that matches
  // the pattern.
  int num_matched = 0;

  // A globstar pattern matches zero or more directories.
  if (_pattern == "**") {
    // Try to match this directory (as if the globstar wasn't there)
    if (suffix.empty()) {
      // This is a directory.  Add it.
      results.push_back(Filename(prefix));
      num_matched++;
    } else if (suffix == "/") {
      // Keep the trailing slash, but be sure not to duplicate it.
      results.push_back(Filename(prefix, ""));
      num_matched++;
    } else {
      num_matched += next_glob.r_match_files(prefix, next_suffix, results, cwd);
    }
    next_suffix = suffix;
    next_glob = *this;
  }

  for (const string &local_file : dir_files) {
    if (_pattern[0] == '.' || (local_file.empty() || local_file[0] != '.')) {
      if (matches(local_file)) {
        // We have a match; continue.
        if (Filename(parent_dir, local_file).is_directory()) {
          if (suffix.empty() && _pattern != "**") {
            results.push_back(Filename(prefix, local_file));
            num_matched++;
          } else if (suffix == "/" && _pattern != "**") {
            results.push_back(Filename(prefix, local_file + "/"));
            num_matched++;
          } else {
            num_matched += next_glob.r_match_files(Filename(prefix, local_file),
                                                   next_suffix, results, cwd);
          }
        } else if (suffix.empty()) {
          results.push_back(Filename(prefix, local_file));
          num_matched++;
        }
      }
    }
  }

  return num_matched;
}

/**
 * Treats the GlobPattern as a filename pattern, and returns true if the given
 * filename matches the pattern.  Unlike matches(), this will not match slash
 * characters for single asterisk characters, and it will ignore path
 * components that only contain a dot.
 */
bool GlobPattern::
matches_file(Filename candidate) const {
  if (_pattern.empty()) {
    // Special case.
    return candidate.empty();
  }

  // Either both must be absolute, or both must be relative.
  if ((_pattern[0] != '/') != candidate.is_local()) {
    return false;
  }

  return r_matches_file(_pattern, candidate);
}

/**
 * The recursive implementation of matches_file().
 */
bool GlobPattern::
r_matches_file(const string &pattern, const Filename &candidate) const {
  // Split off the next bit of pattern.
  std::string next_pattern;
  GlobPattern glob;
  glob.set_case_sensitive(get_case_sensitive());
  glob.set_nomatch_chars(get_nomatch_chars());

  bool pattern_end;
  size_t slash = pattern.find('/');
  if (slash == string::npos) {
    glob.set_pattern(pattern);
    pattern_end = true;
  } else {
    glob.set_pattern(pattern.substr(0, slash));
    next_pattern = pattern.substr(slash + 1);
    pattern_end = false;

    if (slash == 0 || (slash == 1 && pattern[0] == '.')) {
      // Ignore // and /./ in patterns
      return r_matches_file(next_pattern, candidate);
    }
  }

  // Also split off the next component in the candidate filename.
  std::string part;
  Filename next_candidate;

  bool candidate_end;
  size_t fn_slash = ((const std::string &)candidate).find('/');
  if (fn_slash == string::npos) {
    part = candidate;
    candidate_end = true;
  } else {
    part = candidate.substr(0, fn_slash);
    next_candidate = candidate.substr(fn_slash + 1);
    candidate_end = false;

    // Ignore // and /./ in filenames.
    if (fn_slash == 0 || part == ".") {
      return r_matches_file(pattern, next_candidate);
    }
  }

  // Now check if the current part matches the current pattern.
  bool part_matches;
  if (glob.get_pattern() == "**") {
    // This matches any number of parts.
    if (pattern_end) {
      // We might as well stop checking here; it matches whatever might come.
      return true;
    }
    // We branch out to three options: either we match nothing, we match this
    // part only, or we match this part and maybe more.
    return r_matches_file(next_pattern, candidate)
        || (!candidate_end && r_matches_file(next_pattern, next_candidate))
        || (!candidate_end && r_matches_file(pattern, next_candidate));
  }
  else if (glob.get_pattern() == "*" && _nomatch_chars.empty()) {
    // Matches any part (faster version of below)
    part_matches = true;
  }
  else if ((glob.get_pattern() == "." && part.empty())
        || (glob.get_pattern().empty() && part == ".")) {
    // So that /path/. matches /path/, and vice versa.
    part_matches = true;
  }
  else if (glob.has_glob_characters()) {
    part_matches = glob.matches(part);
  }
  else if (get_case_sensitive()) {
    part_matches = (part == glob.get_pattern());
  }
  else {
    part_matches = (cmp_nocase(part, glob.get_pattern()) == 0);
  }

  if (!part_matches) {
    // It doesn't match, so we end our search here.
    return false;
  }

  if (candidate_end && pattern_end) {
    // We've reached the end of both candidate and pattern, so it matches.
    return true;
  }

  if (pattern_end != candidate_end) {
    // One of them has ended, but the other hasn't, so it's not a match.
    return false;
  }

  // It matches; move on to the next part.
  return r_matches_file(next_pattern, next_candidate);
}

/**
 * The recursive implementation of matches().  This returns true if the
 * pattern substring [pi, pend) matches the candidate substring [ci, cend),
 * false otherwise.
 */
bool GlobPattern::
matches_substr(string::const_iterator pi, string::const_iterator pend,
               string::const_iterator ci, string::const_iterator cend) const {
  // If we run out of pattern or candidate string, it's a match only if they
  // both ran out at the same time.
  if (pi == pend || ci == cend) {
    // A special exception: we allow ci to reach the end before pi, only if pi
    // is one character before the end and that last character is '*'.
    if ((ci == cend) && (std::distance(pi, pend) == 1) && (*pi) == '*') {
      return true;
    }
    return (pi == pend && ci == cend);
  }

  switch (*pi) {

  case '*':
    // A '*' in the pattern string means to match any sequence of zero or more
    // characters in the candidate string.  This means we have to recurse
    // twice: either consume one character of the candidate string and
    // continue to try matching the *, or stop trying to match the * here.
    if (_nomatch_chars.find(*ci) == string::npos) {
      return
        matches_substr(pi, pend, ci + 1, cend) ||
        matches_substr(pi + 1, pend, ci, cend);
    } else {
      // On the other hand, if this is one of the nomatch chars, we can only
      // stop here.
      return matches_substr(pi + 1, pend, ci, cend);
    }

  case '?':
    // A '?' in the pattern string means to match exactly one character in the
    // candidate string.  That's easy.
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


/**
 * Called when an unescaped open square bracked is scanned, this is called
 * with pi positioned after the opening square bracket, scans the set
 * sequence, leaving pi positioned on the closing square bracket, and returns
 * true if the indicated character matches the set of characters indicated,
 * false otherwise.
 */
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
