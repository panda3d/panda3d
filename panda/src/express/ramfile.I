/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ramfile.I
 * @author mike
 * @date 1997-01-09
 */

/**
 *
 */
INLINE Ramfile::
Ramfile() {
  _pos = 0;
}

/**
 * Moves the data pointer to the indicated byte position.  It is not an error
 * to move the pointer past the end of data.
 */
INLINE void Ramfile::
seek(size_t pos) {
  _pos = pos;
}

/**
 * Returns the current data pointer position as a byte offset from the
 * beginning of the stream.
 */
INLINE size_t Ramfile::
tell() const {
  return _pos;
}

/**
 * Returns the entire buffer contents as a string, regardless of the current
 * data pointer.
 */
INLINE const std::string &Ramfile::
get_data() const {
  return _data;
}

/**
 * Returns the size of the entire buffer contents.
 */
INLINE size_t Ramfile::
get_data_size() const {
  return _data.size();
}

/**
 * Empties the current buffer contents.
 */
INLINE void Ramfile::
clear() {
  _data.clear();
}
