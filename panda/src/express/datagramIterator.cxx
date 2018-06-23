/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramIterator.cxx
 * @author jns
 * @date 2000-02-07
 */

#include "datagramIterator.h"
#include "pnotify.h"

using std::string;
using std::wstring;

TypeHandle DatagramIterator::_type_handle;

/**
 * Extracts a variable-length string.
 */
string DatagramIterator::
get_string() {
  // First, get the length of the string
  uint16_t s_len = get_uint16();

  nassertr(_datagram != nullptr, "");
  nassertr(_current_index + s_len <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  size_t last_index = _current_index;

  _current_index += s_len;

  return string(ptr + last_index, s_len);
}

/**
 * Extracts a variable-length string with a 32-bit length field.
 */
string DatagramIterator::
get_string32() {
  // First, get the length of the string
  uint32_t s_len = get_uint32();

  nassertr(_datagram != nullptr, "");
  nassertr(_current_index + s_len <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  size_t last_index = _current_index;

  _current_index += s_len;

  return string(ptr + last_index, s_len);
}

/**
 * Extracts a variable-length string, as a NULL-terminated string.
 */
string DatagramIterator::
get_z_string() {
  nassertr(_datagram != nullptr, "");

  // First, determine the length of the string.
  const char *ptr = (const char *)_datagram->get_data();
  size_t length = _datagram->get_length();
  size_t p = _current_index;
  while (p < length && ptr[p] != '\0') {
    ++p;
  }
  nassertr(p < length, "");  // no NULL character?

  size_t last_index = _current_index;
  _current_index = p + 1;

  return string(ptr + last_index, p - last_index);
}

/**
 * Extracts a fixed-length string.  However, if a zero byte occurs within the
 * string, it marks the end of the string.
 */
string DatagramIterator::
get_fixed_string(size_t size) {
  nassertr(_datagram != nullptr, "");
  nassertr(_current_index + size <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  string s(ptr + _current_index, size);

  _current_index += size;

  size_t zero_byte = s.find('\0');
  return s.substr(0, zero_byte);
}

/**
 * Extracts a variable-length wstring (with a 32-bit length field).
 */
wstring DatagramIterator::
get_wstring() {
  // First, get the length of the string
  uint32_t s_len = get_uint32();

  nassertr(_datagram != nullptr, wstring());
  nassertr(_current_index + s_len * 2 <= _datagram->get_length(), wstring());

  wstring result;
  result.reserve(s_len);
  while (s_len > 0) {
    result += wchar_t(get_uint16());
    --s_len;
  }

  return result;
}

/**
 * Extracts the indicated number of bytes in the datagram and returns them as
 * a string.
 */
vector_uchar DatagramIterator::
extract_bytes(size_t size) {
  nassertr((int)size >= 0, vector_uchar());
  nassertr(_datagram != nullptr, vector_uchar());
  nassertr(_current_index + size <= _datagram->get_length(), vector_uchar());

  const unsigned char *ptr = (const unsigned char *)_datagram->get_data();
  ptr += _current_index;

  _current_index += size;

  return vector_uchar(ptr, ptr + size);
}

/**
 * Extracts the indicated number of bytes in the datagram into the given
 * character buffer.  Assumes that the buffer is big enough to hold the
 * requested number of bytes.  Returns the number of bytes that were
 * successfully written.
 */
size_t DatagramIterator::
extract_bytes(unsigned char *into, size_t size) {
  nassertr((int)size >= 0, 0);
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index + size <= _datagram->get_length(), 0);

  const char *ptr = (const char *)_datagram->get_data();
  memcpy(into, ptr + _current_index, size);

  _current_index += size;
  return size;
}

/**
 * Write a string representation of this instance to <out>.
 */
void DatagramIterator::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<""<<"DatagramIterator";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void DatagramIterator::
write(std::ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""<<"DatagramIterator:\n";
  out.width(indent+2); out<<""<<"_current_index "<<_current_index;
  if (_datagram) {
    out<<""<<" (of "<<(get_datagram().get_length())<<")";
    out<<""<<" / 0x"<<(void*)_current_index<<" (of 0x"
      <<(void*)(get_datagram().get_length())<<")\n";
    get_datagram().write(out, indent+2);
  } else {
    out<<""<<" (_datagram is null)\n";
  }
  #endif //] NDEBUG
}
