// Filename: datagramIterator.cxx
// Created by:  jns (07Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "express_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "datagramIterator.h"
#include <notify.h>
#endif

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

  string s =
    _datagram->get_message().substr(_current_index, s_len);

  nassertr(s.length() == s_len, "");
  _current_index += s_len;

  return s;
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

  string s =
    _datagram->get_message().substr(_current_index, size);
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
  int start = _current_index;

  _current_index += size;
  return _datagram->get_message().substr(start, size);
}

