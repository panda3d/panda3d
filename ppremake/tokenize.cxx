// Filename: tokenize.cxx
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "tokenize.h"

#include <ctype.h>

////////////////////////////////////////////////////////////////////
//     Function: tokenize
//  Description: Chops the source string up into pieces delimited by
//               any of the characters specified in delimiters.
//               Repeated delimiter characters represent zero-length
//               tokens.
//
//               It is the user's responsibility to ensure the output
//               vector is cleared before calling this function; the
//               results will simply be appended to the end of the
//               vector.
////////////////////////////////////////////////////////////////////
void
tokenize(const string &source, vector<string> &tokens,
         const string &delimiters) {
  size_t p = 0;
  while (p < source.length()) {
    size_t q = source.find_first_of(delimiters, p);
    if (q == string::npos) {
      tokens.push_back(source.substr(p));
      return;
    }
    tokens.push_back(source.substr(p, q - p));
    p = q + 1;
  }
  tokens.push_back(string());
}

////////////////////////////////////////////////////////////////////
//     Function: tokenize_whitespace
//  Description: Chops the source string up into pieces delimited by
//               whitespace characters.  It is different from
//               tokenize() in that repeated whitespace characters are
//               not significant.
//
//               It is the user's responsibility to ensure the output
//               vector is cleared before calling this function; the
//               results will simply be appended to the end of the
//               vector.
////////////////////////////////////////////////////////////////////
void
tokenize_whitespace(const string &source, vector<string> &tokens) {
  // First, start at the first non-whitespace character.
  size_t p = 0;
  while (p < source.length() && isspace(source[p])) {
    p++;
  }

  while (p < source.length()) {
    // Now scan to the end of the word.
    size_t q = p;
    while (q < source.length() && !isspace(source[q])) {
      q++;
    }
    tokens.push_back(source.substr(p, q - p));
    p = q;

    while (p < source.length() && isspace(source[p])) {
      p++;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: repaste
//  Description: Returns a string representing the given sequence of
//               tokens concatenated together with the separator
//               string between them.
////////////////////////////////////////////////////////////////////
string
repaste(const vector<string> &tokens, const string &separator) {
  string result;
  if (!tokens.empty()) {
    vector<string>::const_iterator ti;
    ti = tokens.begin();
    result += (*ti);
    ++ti;

    while (ti != tokens.end()) {
      result += separator;
      result += (*ti);
      ++ti;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: trim_blanks
//  Description: Returns a new string, equivalent to the source
//               string, but with the leading and trailing whitespace
//               removed.
////////////////////////////////////////////////////////////////////
string
trim_blanks(const string &str) {
  size_t p = 0;
  while (p < str.length() && isspace(str[p])) {
    p++;
  }
 
  size_t q = str.length();
  while (q > p && isspace(str[q - 1])) {
    q--;
  }

  return str.substr(p, q - p);
}

////////////////////////////////////////////////////////////////////
//     Function: contains_whitespace
//  Description: Returns true if the string contains any whitespace
//               characters, false if it does not.
////////////////////////////////////////////////////////////////////
bool
contains_whitespace(const string &str) {
  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    if (isspace(*si)) {
      return true;
    }
  }

  return false;
}
