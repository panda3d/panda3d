/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogate_datafile.cxx
 * @author drose
 * @date 2000-08-09
 */

#include "interrogate_datafile.h"

using std::istream;
using std::ostream;
using std::string;


/**
 * Writes the indicated string to the output file.  Uses the given whitespace
 * character to separate the string's length and its contents.
 */
void
idf_output_string(ostream &out, const string &str, char whitespace) {
  out << str.length() << whitespace;
  if (!str.empty()) {
    out << str << whitespace;
  }
}

/**
 * Reads the given string from the input file, as previously written by
 * output_string().
 */
void
idf_input_string(istream &in, string &str) {
  int length;
  in >> length;
  if (in.fail()) {
    return;
  }

  // Skip one character of whitespace, and then read the string.
  in.get();
  str = "";
  while (length > 0) {
    str += in.get();
    length--;
  }
}

/**
 * Writes the indicated string to the output file.  Uses the given whitespace
 * character to separate the string's length and its contents.
 */
void
idf_output_string(ostream &out, const char *str, char whitespace) {
  if (str == nullptr) {
    out << "0 ";
  } else {
    out << strlen(str) << whitespace;
    if (str[0] != '\0') {
      out << str << whitespace;
    }
  }
}

/**
 * Reads the given string from the input file, as previously written by
 * output_string().
 */
void
idf_input_string(istream &in, const char *&str) {
  int length;
  in >> length;
  if (in.fail()) {
    return;
  }

  if (length == 0) {
    // Don't change the string if the input length is zero.
    return;
  }

  // Skip one character of whitespace, and then read the string.
  in.get();
  char *readstr = new char[length + 1];
  int p = 0;
  while (p < length) {
    readstr[p] = in.get();
    p++;
  }
  readstr[p] = '\0';

  str = readstr;
}
