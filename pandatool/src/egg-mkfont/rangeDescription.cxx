/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rangeDescription.cxx
 * @author drose
 * @date 2003-09-07
 */

#include "rangeDescription.h"
#include "string_utils.h"
#include "pnotify.h"

using std::string;

/**
 *
 */
RangeDescription::
RangeDescription() {
}

/**
 * Parses a string of comma- and hyphen-delimited unicode values, in decimal
 * and/or hex, including possible bracket-delimited ASCII characters, as may
 * have been passed on a command line.  Returns true if the parameter is
 * parsed correctly, false otherwise.
 */
bool RangeDescription::
parse_parameter(const string &param) {
  // First, go through and separate the string by commas.  We have to do this
  // by hand instead of calling tokenize(), because we also have to scan for
  // square brackets, which may contain nested commas.
  size_t p = 0;
  while (p < param.length()) {
    size_t q = param.find_first_of("[,", p);
    if (q == string::npos) {
      return parse_word(trim(param.substr(p)));
    }
    if (!parse_word(trim(param.substr(p, q - p)))) {
      return false;
    }

    if (param[q] == '[') {
      // A square bracket means we must search for the matching square
      // bracket.  However, a right bracket immediately after the left bracket
      // doesn't count; we start the scan after that.
      p = param.find("]", q + 2);
      if ( p == string::npos) {
        nout << "Unclosed open bracket.\n";
        return false;
      }
      if (!parse_bracket(param.substr(q + 1, p - q - 1))) {
        return false;
      }
      p = p + 1;

    } else {
      // Otherwise, if the separator was just a comma, the next character
      // begins the next word.
      p = q + 1;
    }
  }

  return true;
}

/**
 *
 */
void RangeDescription::
output(std::ostream &out) const {
  bool first_time = true;
  RangeList::const_iterator ri;
  for (ri = _range_list.begin(); ri != _range_list.end(); ++ri) {
    const Range &range = (*ri);
    if (!first_time) {
      out << ",";
    }
    first_time = false;
    if (range._from_code == range._to_code) {
      out << range._from_code;
    } else {
      out << range._from_code << "-" << range._to_code;
    }
  }
}

/**
 * Parses a single "word", i.e.  the text delimited by commas, that might be
 * listed on the command line.  This is generally either the empty string, a
 * single number, or a pair of numbers separated by a hyphen.
 */
bool RangeDescription::
parse_word(const string &word) {
  if (word.empty()) {
    return true;
  }

  // It's not empty, so see if it includes a hyphen.
  size_t hyphen = word.find('-');
  if (hyphen == string::npos) {
    // Nope, just one number.
    int code;
    if (!parse_code(word, code)) {
      return false;
    }
    add_singleton(code);

  } else {
    // Two numbers separated by a hyphen.
    int from_code, to_code;
    if (!parse_code(word.substr(0, hyphen), from_code)) {
      return false;
    }
    if (!parse_code(word.substr(hyphen + 1), to_code)) {
      return false;
    }
    add_range(from_code, to_code);
  }

  return true;
}

/**
 * Parses a single numeric value, either decimal or hexadecimal, and stores it
 * in the indicated parameter.  Returns true if successful, false otherwise.
 */
bool RangeDescription::
parse_code(const string &word, int &code) {
  string str = trim(word);
  const char *nptr = str.c_str();
  char *endptr;
  code = strtol(nptr, &endptr, 0);
  if (*endptr == '\0') {
    return true;
  }

  nout << "Invalid Unicode value: " << word << "\n";
  return false;
}

/**
 * Parses the text listed between square brackets on the command line.
 */
bool RangeDescription::
parse_bracket(const string &str) {
  string::const_iterator si;
  si = str.begin();
  while (si != str.end()) {
    int ch = (*si);
    ++si;
    if (si != str.end() && (*si) == '-') {
      // A hyphen indicates a range.
      ++si;
      if (si == str.end()) {
        // Unless the hyphen is the last character.
        add_singleton(ch);
        add_singleton('-');
      } else {
        add_range(ch, (*si));
        ++si;
      }
    } else {
      // Anything other than a hyphen indicates a singleton.
      add_singleton(ch);
    }
  }

  return true;
}
