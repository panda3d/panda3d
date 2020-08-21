/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramBuffer.I
 * @author rdb
 * @date 2017-11-07
 */

/**
 * Initializes an empty datagram buffer.
 */
INLINE DatagramBuffer::
DatagramBuffer() :
  _read_offset(0),
  _wrote_first_datagram(false),
  _read_first_datagram(false) {
}

/**
 * Initializes the buffer with the given data.
 */
INLINE DatagramBuffer::
DatagramBuffer(vector_uchar data) :
  _data(std::move(data)),
  _read_offset(0),
  _wrote_first_datagram(false),
  _read_first_datagram(false) {
}

/**
 * Clears the internal buffer.
 */
INLINE void DatagramBuffer::
clear() {
  _data.clear();
  _read_offset = 0;
  _wrote_first_datagram = false;
  _read_first_datagram = false;
}

/**
 * Returns the internal buffer.
 */
INLINE const vector_uchar &DatagramBuffer::
get_data() const {
  return _data;
}

/**
 * Replaces the data in the internal buffer.
 */
INLINE void DatagramBuffer::
set_data(vector_uchar data) {
  _data = std::move(data);
}

/**
 * Swaps the data in the internal buffer with that of the other buffer.
 */
INLINE void DatagramBuffer::
swap_data(vector_uchar &other) {
  _data.swap(other);
}
