/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamWriter.cxx
 * @author drose
 * @date 2002-08-04
 */

#include "streamWriter.h"

/**
 * Adds the indicated number of zero bytes to the stream.
 */
void StreamWriter::
pad_bytes(size_t size) {
  nassertv((int)size >= 0);

  while (size > 0) {
    _out->put('\0');
    size--;
  }
}
