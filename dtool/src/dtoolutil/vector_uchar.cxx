/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vector_uchar.cxx
 * @author drose
 * @date 2000-05-10
 */

#include "vector_uchar.h"

#define EXPCL EXPCL_DTOOL_DTOOLUTIL
#define EXPTP EXPTP_DTOOL_DTOOLUTIL
#define TYPE unsigned char
#define NAME vector_uchar

#include "vector_src.cxx"

/**
 * Writes a hex representation of the bytes to the output stream.
 */
std::ostream &
operator << (std::ostream &out, const vector_uchar &data) {
  static const char nibble_to_hex[] = "0123456789abcdef";

  if (data.empty()) {
    return out;
  }

  out << nibble_to_hex[(data[0] & 0xf0) >> 4];
  out << nibble_to_hex[data[0] & 0xf];

  for (size_t i = 1; i < data.size(); ++i) {
    out << ' ';
    out << nibble_to_hex[(data[i] & 0xf0) >> 4];
    out << nibble_to_hex[data[i] & 0xf];
  }
  return out;
}
