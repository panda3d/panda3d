/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ramfile.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "ramfile.h"

/**
 * Extracts and returns the indicated number of characters from the current
 * data pointer, and advances the data pointer.  If the data pointer exceeds
 * the end of the buffer, returns empty string.
 *
 * The interface here is intentionally designed to be similar to that for
 * Python's file.read() function.
 */
std::string Ramfile::
read(size_t length) {
  size_t orig_pos = _pos;
  _pos = std::min(_pos + length, _data.length());
  return _data.substr(orig_pos, length);
}

/**
 * Assumes the stream represents a text file, and extracts one line up to and
 * including the trailing newline character.  Returns empty string when the
 * end of file is reached.
 *
 * The interface here is intentionally designed to be similar to that for
 * Python's file.readline() function.
 */
std::string Ramfile::
readline() {
  size_t start = _pos;
  while (_pos < _data.length() && _data[_pos] != '\n') {
    ++_pos;
  }

  if (_pos < _data.length() && _data[_pos] == '\n') {
    // Include the newline character also.
    ++_pos;
  }

  return _data.substr(start, _pos - start);
}
