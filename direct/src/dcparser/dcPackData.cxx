/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPackData.cxx
 * @author drose
 * @date 2004-06-15
 */

#include "dcPackData.h"

static const size_t extra_size = 50;

/**
 * Ensures that the buffer has at least size bytes, and sets the _used_length
 * to the indicated value; grows the buffer if it does not.
 */
void DCPackData::
set_used_length(size_t size) {
  if (size > _allocated_size) {
    _allocated_size = size + size + extra_size;
    char *new_buf = new char[_allocated_size];
    if (_used_length > 0) {
      memcpy(new_buf, _buffer, _used_length);
    }
    if (_buffer != nullptr) {
      delete[] _buffer;
    }
    _buffer = new_buf;
  }

  _used_length = size;
}
