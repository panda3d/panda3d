/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file copy_stream.cxx
 * @author drose
 * @date 2009-08-27
 */

#include "copy_stream.h"

/**
 * Reads the source stream from its current position to the end of the stream,
 * and writes that data to the dest stream at its current position.  Returns
 * true on success, false on failure.
 */
bool
copy_stream(std::istream &source, std::ostream &dest) {
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  source.read(buffer, buffer_size);
  size_t count = source.gcount();
  while (count != 0) {
    dest.write(buffer, count);
    source.read(buffer, buffer_size);
    count = source.gcount();
  }

  return (!source.fail() || source.eof()) && (!dest.fail());
}
