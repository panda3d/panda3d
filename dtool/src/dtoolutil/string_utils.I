/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file string_utils.I
 * @author drose
 * @date 2000-07-14
 */

template<class Thing>
INLINE string
format_string(const Thing &thing) {
  ostringstream str;
  str << thing;
  return str.str();
}

INLINE string
format_string(const string &value) {
  return value;
}

INLINE string
format_string(bool value) {
  return string(value ? "true" : "false");
}

INLINE string
format_string(float value) {
  char buffer[32];
  pdtoa(value, buffer);
  return string(buffer);
}

INLINE string
format_string(double value) {
  char buffer[32];
  pdtoa(value, buffer);
  return string(buffer);
}

INLINE string
format_string(unsigned int value) {
  char buffer[11];
  char *p = buffer + 10;
  *p = 0;
  do {
    *--p = '0' + (value % 10);
    value /= 10;
  } while (value > 0);

  return string(p);
}

INLINE string
format_string(int value) {
  char buffer[12];
  char *p = buffer + 11;
  *p = 0;

  if (value < 0) {
    unsigned int posv = (unsigned int)-value;
    do {
      *--p = '0' + (posv % 10);
      posv /= 10;
    } while (posv > 0);
    *--p = '-';
  } else {
    do {
      *--p = '0' + (value % 10);
      value /= 10;
    } while (value > 0);
  }

  return string(p);
}

INLINE string
format_string(int64_t value) {
  char buffer[21];
  char *p = buffer + 20;
  *p = 0;

  if (value < 0) {
    uint64_t posv = (uint64_t)-value;
    do {
      *--p = '0' + (posv % 10);
      posv /= 10;
    } while (posv > 0);
    *--p = '-';
  } else {
    do {
      *--p = '0' + (value % 10);
      value /= 10;
    } while (value > 0);
  }

  return string(p);
}
