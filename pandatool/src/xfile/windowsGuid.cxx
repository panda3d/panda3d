/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowsGuid.cxx
 * @author drose
 * @date 2004-10-03
 */

#include "windowsGuid.h"
#include "pnotify.h"

#include <stdio.h>  // for sscanf, sprintf

using std::string;

/**
 * Parses the hex representation in the indicated string and stores it in the
 * WindowsGuid object.  Returns true if successful, false if the string
 * representation is malformed.
 */
bool WindowsGuid::
parse_string(const string &str) {
  unsigned long data1;
  unsigned int data2, data3;
  unsigned int b1, b2, b3, b4, b5, b6, b7, b8;
  int result = sscanf(str.c_str(),
                      "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                      &data1, &data2, &data3,
                      &b1, &b2, &b3, &b4, &b5, &b6, &b7, &b8);
  if (result != 11) {
    return false;
  }

  _data1 = data1;
  _data2 = data2;
  _data3 = data3;
  _b1 = b1;
  _b2 = b2;
  _b3 = b3;
  _b4 = b4;
  _b5 = b5;
  _b6 = b6;
  _b7 = b7;
  _b8 = b8;

  return true;
}

/**
 * Returns a hex representation of the GUID.
 */
string WindowsGuid::
format_string() const {
  static const int buf_length = 128;  // Actually, we only need 36 + 1 == 37.
  char buffer[buf_length];
  sprintf(buffer,
          "%08lx-%04hx-%04hx-%02x%02x-%02x%02x%02x%02x%02x%02x",
          _data1, _data2, _data3,
          _b1, _b2, _b3, _b4, _b5, _b6, _b7, _b8);
  nassertr((int)strlen(buffer) < buf_length, string());

  return string(buffer);
}

/**
 * Outputs a hex representation of the GUID.
 */
void WindowsGuid::
output(std::ostream &out) const {
  out << format_string();
}
