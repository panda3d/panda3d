/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringStream.I
 * @author drose
 * @date 2007-07-03
 */

/**
 *
 */
INLINE StringStream::
StringStream() : std::iostream(&_buf) {
}

/**
 * This version of the constructor preloads the buffer with the indicated
 * data.
 */
INLINE StringStream::
StringStream(const std::string &source) : std::iostream(&_buf) {
  set_data(source);
}

/**
 * This version of the constructor preloads the buffer with the indicated
 * data.
 */
INLINE StringStream::
StringStream(vector_uchar source) : std::iostream(&_buf) {
  _buf.swap_data(source);
}

/**
 * Empties the buffer.
 */
INLINE void StringStream::
clear_data() {
  _buf.clear();
}

/**
 * Returns the number of characters available to be read from the data stream.
 */
INLINE size_t StringStream::
get_data_size() {
  flush();
  return _buf.get_data().size();
}

/**
 * Returns the contents of the data stream as a string.
 */
INLINE std::string StringStream::
get_data() {
  flush();
  const vector_uchar &data = _buf.get_data();
  if (!data.empty()) {
    return std::string((char *)&data[0], data.size());
  }
  return std::string();
}

/**
 * Replaces the contents of the data stream.  This implicitly reseeks to 0.
 */
INLINE void StringStream::
set_data(const std::string &data) {
  _buf.clear();
  if (!data.empty()) {
    set_data((const unsigned char *)data.data(), data.size());
  }
}

/**
 * Swaps the indicated buffer for the contents of the internal buffer.
 */
INLINE void StringStream::
swap_data(vector_uchar &data) {
  flush();
  _buf.swap_data(data);
}
