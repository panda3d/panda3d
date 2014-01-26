// Filename: datagramIterator.cxx
// Created by:  jns (07Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "datagramIterator.h"
#include "pnotify.h"

TypeHandle DatagramIterator::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_string
//       Access: Public
//  Description: Extracts a variable-length string.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
get_string() {
  // First, get the length of the string
  PN_uint16 s_len = get_uint16();

  nassertr(_datagram != (const Datagram *)NULL, "");
  nassertr(_current_index + s_len <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  int last_index = _current_index;

  _current_index += s_len;

  return string(ptr + last_index, s_len);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_string32
//       Access: Public
//  Description: Extracts a variable-length string with a 32-bit
//               length field.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
get_string32() {
  // First, get the length of the string
  PN_uint32 s_len = get_uint32();

  nassertr(_datagram != (const Datagram *)NULL, "");
  nassertr(_current_index + s_len <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  int last_index = _current_index;

  _current_index += s_len;

  return string(ptr + last_index, s_len);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_z_string
//       Access: Public
//  Description: Extracts a variable-length string, as a
//               NULL-terminated string.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
get_z_string() {
  nassertr(_datagram != (const Datagram *)NULL, "");

  // First, determine the length of the string.
  const char *ptr = (const char *)_datagram->get_data();
  size_t length = _datagram->get_length();
  size_t p = _current_index;
  while (p < length && ptr[p] != '\0') {
    ++p;
  }
  nassertr(p < length, "");  // no NULL character?

  int last_index = _current_index;
  _current_index = p + 1;

  return string(ptr + last_index, p - last_index);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_fixed_string
//       Access: Public
//  Description: Extracts a fixed-length string.  However, if a zero
//               byte occurs within the string, it marks the end of
//               the string.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
get_fixed_string(size_t size) {
  nassertr(_datagram != (const Datagram *)NULL, "");
  nassertr(_current_index + size <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  string s(ptr + _current_index, size);

  _current_index += size;

  size_t zero_byte = s.find('\0');
  return s.substr(0, zero_byte);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_wstring
//       Access: Public
//  Description: Extracts a variable-length wstring (with a 32-bit
//               length field).
////////////////////////////////////////////////////////////////////
wstring DatagramIterator::
get_wstring() {
  // First, get the length of the string
  PN_uint32 s_len = get_uint32();

  nassertr(_datagram != (const Datagram *)NULL, wstring());
  nassertr(_current_index + s_len * 2 <= _datagram->get_length(), wstring());

  wstring result;
  result.reserve(s_len);
  while (s_len > 0) {
    result += wchar_t(get_uint16());
    --s_len;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::extract_bytes
//       Access: Public
//  Description: Extracts the indicated number of bytes in the
//               datagram and returns them as a string.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
extract_bytes(size_t size) {
  nassertr((int)size >= 0, "");
  nassertr(_datagram != (const Datagram *)NULL, "");
  nassertr(_current_index + size <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  int last_index = _current_index;

  _current_index += size;

  return string(ptr + last_index, size);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::extract_bytes
//       Access: Published
//  Description: Extracts the indicated number of bytes in the
//               datagram into the given character buffer.  Assumes
//               that the buffer is big enough to hold the requested
//               number of bytes.  Returns the number of bytes
//               that were successfully written.
////////////////////////////////////////////////////////////////////
size_t DatagramIterator::
extract_bytes(unsigned char *into, size_t size) {
  nassertr((int)size >= 0, 0);
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index + size <= _datagram->get_length(), 0);

  const char *ptr = (const char *)_datagram->get_data();
  memcpy(into, ptr + _current_index, size);

  _current_index += size;
  return size;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void DatagramIterator::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<""<<"DatagramIterator";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void DatagramIterator::
write(ostream &out, unsigned int indent) const {
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

