// Filename: ppFilenamePattern.h
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPFILENAMEPATTERN_H
#define PPFILENAMEPATTERN_H

#include "ppremake.h"

///////////////////////////////////////////////////////////////////
//       Class : PPFilenamePattern
// Description : This is a string that represents a filename, or a
//               family of filenames, using the make convention that a
//               wildcard sign (PATTERN_WILDCARD, typically '%') in
//               the filename represents any sequence of characters.
////////////////////////////////////////////////////////////////////
class PPFilenamePattern {
public:
  PPFilenamePattern(const string &pattern);
  PPFilenamePattern(const PPFilenamePattern &copy);
  void operator = (const PPFilenamePattern &copy);

  bool has_wildcard() const;
  string get_pattern() const;
  const string &get_prefix() const;
  const string &get_suffix() const;

  bool matches(const string &filename) const;
  string extract_body(const string &filename) const;
  string transform(const string &filename,
                   const PPFilenamePattern &transform_from) const;

private:
  bool _has_wildcard;
  string _prefix;
  string _suffix;
};

inline ostream &
operator << (ostream &out, const PPFilenamePattern &pattern) {
  return out << pattern.get_pattern();
}

#endif
