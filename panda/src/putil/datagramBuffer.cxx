/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramBuffer.cxx
 * @author rdb
 * @date 2017-11-07
 */

#include "datagramBuffer.h"

/**
 * Writes a sequence of bytes to the beginning of the datagram file.  This may
 * be called any number of times after the file has been opened and before the
 * first datagram is written.  It may not be called once the first datagram is
 * written.
 */
bool DatagramBuffer::
write_header(const std::string &header) {
  nassertr(!_wrote_first_datagram, false);

  _data.insert(_data.end(), header.begin(), header.end());
  return true;
}

/**
 * Writes the given datagram to the file.  Returns true on success, false if
 * there is an error.
 */
bool DatagramBuffer::
put_datagram(const Datagram &data) {
  _wrote_first_datagram = true;

  // First, write the size of the upcoming datagram.
  size_t num_bytes = data.get_length();
  size_t offset = _data.size();

  if (num_bytes == (uint32_t)-1 || num_bytes != (uint32_t)num_bytes) {
    // Write a large value as a 64-bit size.
    _data.resize(offset + num_bytes + 4 + sizeof(uint64_t));
    _data[offset++] = 0xff;
    _data[offset++] = 0xff;
    _data[offset++] = 0xff;
    _data[offset++] = 0xff;

    LittleEndian s(&num_bytes, sizeof(uint64_t));
    memcpy(&_data[offset], s.get_data(), sizeof(uint64_t));
    offset += sizeof(uint64_t);
  } else {
    // Write a value that fits in 32 bits.
    _data.resize(offset + num_bytes + sizeof(uint32_t));

    LittleEndian s(&num_bytes, sizeof(uint32_t));
    memcpy(&_data[offset], s.get_data(), sizeof(uint32_t));
    offset += sizeof(uint32_t);
  }

  // Now, write the datagram itself.
  memcpy(&_data[offset], data.get_data(), data.get_length());
  return true;
}

/**
 * This does absolutely nothing.
 */
void DatagramBuffer::
flush() {
}

/**
 * Reads a sequence of bytes from the beginning of the datagram file.  This
 * may be called any number of times after the file has been opened and before
 * the first datagram is read.  It may not be called once the first datagram
 * has been read.
 */
bool DatagramBuffer::
read_header(std::string &header, size_t num_bytes) {
  nassertr(!_read_first_datagram, false);
  if (_read_offset + num_bytes > _data.size()) {
    return false;
  }

  header = std::string((char *)&_data[_read_offset], num_bytes);
  _read_offset += num_bytes;
  return true;
}

/**
 * Reads the next datagram from the file.  Returns true on success, false if
 * there is an error or end of file.
 */
bool DatagramBuffer::
get_datagram(Datagram &data) {
  _read_first_datagram = true;
  if (_read_offset + sizeof(uint32_t) > _data.size()) {
    // Reached the end of the buffer.
    return false;
  }

  // First, get the size of the upcoming datagram.
  uint32_t num_bytes_32;
  LittleEndian s(&_data[_read_offset], 0, sizeof(uint32_t));
  s.store_value(&num_bytes_32, sizeof(uint32_t));
  _read_offset += 4;

  if (num_bytes_32 == 0) {
    // A special case for a zero-length datagram: no need to try to read any
    // data.
    data.clear();
    return true;
  }

  size_t num_bytes = (size_t)num_bytes_32;
  if (num_bytes_32 == (uint32_t)-1) {
    // Another special case for a value larger than 32 bits.
    uint64_t num_bytes_64;
    LittleEndian s(&_data[_read_offset], 0, sizeof(uint64_t));
    s.store_value(&num_bytes_64, sizeof(uint64_t));
    _read_offset += 8;

    num_bytes = (size_t)num_bytes_64;
    nassertr((uint64_t)num_bytes == num_bytes_64, false);
  }

  // Make sure we have this much data to read.
  nassertr_always(_read_offset + num_bytes <= _data.size(), false);

  data = Datagram(&_data[_read_offset], num_bytes);
  _read_offset += num_bytes;
  return true;
}

/**
 * Returns true if the buffer has reached the end-of-buffer.  This test may
 * only be made after a call to read_header() or get_datagram() has failed.
 */
bool DatagramBuffer::
is_eof() {
  return (_read_offset + sizeof(uint32_t)) > _data.size();
}

/**
 * Returns true if the buffer has reached an error condition.
 */
bool DatagramBuffer::
is_error() {
  return false;
}
