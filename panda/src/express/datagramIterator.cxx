// Filename: datagramIterator.cxx
// Created by:  jns (07Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


#include "datagramIterator.h"
#include "notify.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_string
//       Access: Public
//  Description: Extracts a variable-length string.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
get_string() {
  // First, get the length of the string
  PN_uint16 s_len = get_uint16();

  nassertr(_datagram != (const Datagram *)NULL &&
           _current_index + s_len <= _datagram->get_length(), "");

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

  nassertr(_datagram != (const Datagram *)NULL &&
           _current_index + s_len <= _datagram->get_length(), "");

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
  nassertr(_datagram != (const Datagram *)NULL &&
           _current_index + size <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  string s(ptr + _current_index, size);

  _current_index += size;

  size_t zero_byte = s.find('\0');
  return s.substr(0, zero_byte);
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
  nassertr(_datagram != (const Datagram *)NULL &&
           _current_index + size <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  int last_index = _current_index;

  _current_index += size;

  return string(ptr + last_index, size);
}

