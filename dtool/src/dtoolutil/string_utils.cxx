/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file string_utils.cxx
 * @author drose
 * @date 1999-01-18
 */

#include "string_utils.h"
#include "textEncoder.h"
#include "pstrtod.h"

#include <ctype.h>

using std::string;
using std::wstring;

// Case-insensitive string comparison, from Stroustrup's C++ third edition.
// Works like strcmp().
int
cmp_nocase(const string &s, const string &s2) {
  string::const_iterator p = s.begin();
  string::const_iterator p2 = s2.begin();

  while (p != s.end() && p2 != s2.end()) {
    if (toupper(*p) != toupper(*p2)) {
      return (toupper(*p) < toupper(*p2)) ? -1 : 1;
    }
    ++p;
    ++p2;
  }

  return (s2.size() == s.size()) ? 0 :
    (s.size() < s2.size()) ? -1 : 1;  // size is unsigned
}

INLINE int
toupper_uh(int ch) {
  return (ch == '_') ? '-' : toupper(ch);
}


int
cmp_nocase_uh(const string &s, const string &s2) {
  string::const_iterator p = s.begin();
  string::const_iterator p2 = s2.begin();

  while (p != s.end() && p2 != s2.end()) {
    if (toupper_uh(*p) != toupper_uh(*p2)) {
      return (toupper_uh(*p) < toupper_uh(*p2)) ? -1 : 1;
    }
    ++p;
    ++p2;
  }

  return (s2.size() == s.size()) ? 0 :
    (s.size() < s2.size()) ? -1 : 1;  // size is unsigned
}



/**
 * Returns the input string with all uppercase letters converted to lowercase.
 */
string
downcase(const string &s) {
  string result;
  result.reserve(s.size());
  string::const_iterator p;
  for (p = s.begin(); p != s.end(); ++p) {
    result += tolower(*p);
  }
  return result;
}

/**
 * Returns the input string with all lowercase letters converted to uppercase.
 */
string
upcase(const string &s) {
  string result;
  result.reserve(s.size());
  string::const_iterator p;
  for (p = s.begin(); p != s.end(); ++p) {
    result += toupper(*p);
  }
  return result;
}


/**
 * Divides the string into a number of words according to whitespace.  The
 * words vector should be cleared by the user before calling; otherwise, the
 * list of words in the string will be appended to the end of whatever was
 * there before.
 *
 * The return value is the number of words extracted.
 */
int
extract_words(const string &str, vector_string &words) {
  int num_words = 0;

  size_t pos = 0;
  while (pos < str.length() && isspace((unsigned int)str[pos])) {
    pos++;
  }
  while (pos < str.length()) {
    size_t word_start = pos;
    while (pos < str.length() && !isspace((unsigned int)str[pos])) {
      pos++;
    }
    words.push_back(str.substr(word_start, pos - word_start));
    num_words++;

    while (pos < str.length() && isspace((unsigned int)str[pos])) {
      pos++;
    }
  }

  return num_words;
}

/**
 * Divides the string into a number of words according to whitespace.  The
 * words vector should be cleared by the user before calling; otherwise, the
 * list of words in the string will be appended to the end of whatever was
 * there before.
 *
 * The return value is the number of words extracted.
 */
int
extract_words(const wstring &str, pvector<wstring> &words) {
  int num_words = 0;

  size_t pos = 0;
  while (pos < str.length() && TextEncoder::unicode_isspace(str[pos])) {
    pos++;
  }
  while (pos < str.length()) {
    size_t word_start = pos;
    while (pos < str.length() && !TextEncoder::unicode_isspace(str[pos])) {
      pos++;
    }
    words.push_back(str.substr(word_start, pos - word_start));
    num_words++;

    while (pos < str.length() && TextEncoder::unicode_isspace(str[pos])) {
      pos++;
    }
  }

  return num_words;
}

/**
 * Chops the source string up into pieces delimited by any of the characters
 * specified in delimiters.  Repeated delimiter characters represent zero-
 * length tokens.
 *
 * It is the user's responsibility to ensure the output vector is cleared
 * before calling this function; the results will simply be appended to the
 * end of the vector.
 */
void
tokenize(const string &str, vector_string &words, const string &delimiters,
         bool discard_repeated_delimiters) {
  size_t p = 0;
  while (p < str.length()) {
    size_t q = str.find_first_of(delimiters, p);
    if (q == string::npos) {
      if (q - p || !discard_repeated_delimiters){
        words.push_back(str.substr(p));
      }
      return;
    }
    if (q - p || !discard_repeated_delimiters){
        words.push_back(str.substr(p, q - p));
    }
    p = q + 1;
  }
  words.push_back(string());
}

/**
 * Chops the source string up into pieces delimited by any of the characters
 * specified in delimiters.  Repeated delimiter characters represent zero-
 * length tokens.
 *
 * It is the user's responsibility to ensure the output vector is cleared
 * before calling this function; the results will simply be appended to the
 * end of the vector.
 */
void
tokenize(const wstring &str, pvector<wstring> &words, const wstring &delimiters,
         bool discard_repeated_delimiters) {
  size_t p = 0;
  while (p < str.length()) {
    size_t q = str.find_first_of(delimiters, p);
    if (q == string::npos) {
      if (q - p || !discard_repeated_delimiters){
        words.push_back(str.substr(p));
      }
      return;
    }
    if (q - p || !discard_repeated_delimiters){
      words.push_back(str.substr(p, q - p));
    }
    p = q + 1;
  }
  words.push_back(wstring());
}

/**
 * Returns a new string representing the contents of the given string with the
 * leading whitespace removed.
 */
string
trim_left(const string &str) {
  size_t begin = 0;
  while (begin < str.size() && isspace((unsigned int)str[begin])) {
    begin++;
  }

  return str.substr(begin);
}

/**
 * Returns a new string representing the contents of the given string with the
 * leading whitespace removed.
 */
wstring
trim_left(const wstring &str) {
  size_t begin = 0;
  while (begin < str.size() && TextEncoder::unicode_isspace(str[begin])) {
    begin++;
  }

  return str.substr(begin);
}

/**
 * Returns a new string representing the contents of the given string with the
 * trailing whitespace removed.
 */
string
trim_right(const string &str) {
  size_t begin = 0;
  size_t end = str.size();
  while (end > begin && isspace((unsigned int)str[end - 1])) {
    end--;
  }

  return str.substr(begin, end - begin);
}

/**
 * Returns a new string representing the contents of the given string with the
 * trailing whitespace removed.
 */
wstring
trim_right(const wstring &str) {
  size_t begin = 0;
  size_t end = str.size();
  while (end > begin && TextEncoder::unicode_isspace(str[end - 1])) {
    end--;
  }

  return str.substr(begin, end - begin);
}

/**
 * Returns a new string representing the contents of the given string with
 * both leading and trailing whitespace removed.
 */
string
trim(const string &str) {
  size_t begin = 0;
  while (begin < str.size() && isspace((unsigned int)str[begin])) {
    begin++;
  }

  size_t end = str.size();
  while (end > begin && isspace((unsigned int)str[end - 1])) {
    end--;
  }

  return str.substr(begin, end - begin);
}

/**
 * Returns a new string representing the contents of the given string with
 * both leading and trailing whitespace removed.
 */
wstring
trim(const wstring &str) {
  size_t begin = 0;
  while (begin < str.size() && TextEncoder::unicode_isspace(str[begin])) {
    begin++;
  }

  size_t end = str.size();
  while (end > begin && TextEncoder::unicode_isspace(str[end - 1])) {
    end--;
  }

  return str.substr(begin, end - begin);
}

/**
 * A string-interface wrapper around the C library strtol().  This parses the
 * ASCII representation of an integer, and then sets tail to everything that
 * follows the first valid integer read.  If, on exit, str == tail, there was
 * no valid integer in the source string; if !tail.empty(), there was garbage
 * after the integer.
 *
 * It is legal if str and tail refer to the same string.
 */
int
string_to_int(const string &str, string &tail) {
  const char *nptr = str.c_str();
  char *endptr;
  int result = strtol(nptr, &endptr, 10);
  tail = endptr;
  return result;
}

/**
 * Another flavor of string_to_int(), this one returns true if the string is a
 * perfectly valid integer (and sets result to that value), or false
 * otherwise.
 */
bool
string_to_int(const string &str, int &result) {
  string tail;
  result = string_to_int(str, tail);
  return tail.empty();
}

/**
 * A string-interface wrapper around the C library strtol().  This parses the
 * ASCII representation of an floating-point number, and then sets tail to
 * everything that follows the first valid integer read.  If, on exit, str ==
 * tail, there was no valid integer in the source string; if !tail.empty(),
 * there was garbage after the number.
 *
 * It is legal if str and tail refer to the same string.
 */
double
string_to_double(const string &str, string &tail) {
  const char *nptr = str.c_str();
  char *endptr;
  double result = pstrtod(nptr, &endptr);
  tail = endptr;
  return result;
}

/**
 * Another flavor of string_to_double(), this one returns true if the string
 * is a perfectly valid number (and sets result to that value), or false
 * otherwise.
 */
bool
string_to_double(const string &str, double &result) {
  string tail;
  result = string_to_double(str, tail);
  return tail.empty();
}

/**
 *
 */
bool
string_to_float(const string &str, float &result) {
  string tail;
  result = (float)string_to_double(str, tail);
  return tail.empty();
}

/**
 *
 */
bool
string_to_stdfloat(const string &str, PN_stdfloat &result) {
  string tail;
  result = (PN_stdfloat)string_to_double(str, tail);
  return tail.empty();
}
