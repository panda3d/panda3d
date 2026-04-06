/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineStreamBuf.I
 * @author drose
 * @date 2000-02-26
 */

/**
 *
 */
INLINE bool LineStreamBuf::
is_text_available() const {
  return !_data.empty();
}

/**
 *
 */
INLINE bool LineStreamBuf::
has_newline() const {
  return _has_newline;
}

/**
 * An internal function called by sync() and overflow() to store one or more
 * characters written to the stream into the memory buffer.
 */
INLINE void LineStreamBuf::
write_chars(const char *start, size_t length) {
  if (length > 0) {
    _data += std::string(start, length);
  }
}
