// Filename: streamReader.cxx
// Created by:  drose (04Aug02)
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

#include "streamReader.h"


////////////////////////////////////////////////////////////////////
//     Function: StreamReader::get_string
//       Access: Published
//  Description: Extracts a variable-length string.
////////////////////////////////////////////////////////////////////
string StreamReader::
get_string() {
  nassertr(!_in->eof() && !_in->fail(), string());

  // First, get the length of the string
  size_t s_len = get_uint16();

  string result;
  result.reserve(s_len);
  for (size_t p = 0; !_in->eof() && !_in->fail() && p < s_len; p++) {
    result += _in->get();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: StreamReader::get_string32
//       Access: Published
//  Description: Extracts a variable-length string with a 32-bit
//               length field.
////////////////////////////////////////////////////////////////////
string StreamReader::
get_string32() {
  nassertr(!_in->eof() && !_in->fail(), string());

  // First, get the length of the string
  size_t s_len = get_uint32();

  string result;
  result.reserve(s_len);
  for (size_t p = 0; !_in->eof() && !_in->fail() && p < s_len; p++) {
    result += _in->get();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: StreamReader::get_z_string
//       Access: Published
//  Description: Extracts a variable-length string, as a
//               NULL-terminated string.
////////////////////////////////////////////////////////////////////
string StreamReader::
get_z_string() {
  nassertr(!_in->eof() && !_in->fail(), string());

  string result;
  int ch = _in->get();
  while (!_in->eof() && !_in->fail() && ch != '\0') {
    result += ch;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: StreamReader::get_fixed_string
//       Access: Published
//  Description: Extracts a fixed-length string.  However, if a zero
//               byte occurs within the string, it marks the end of
//               the string.
////////////////////////////////////////////////////////////////////
string StreamReader::
get_fixed_string(size_t size) {
  nassertr(!_in->eof() && !_in->fail(), string());

  string result;
  result.reserve(size);
  for (size_t p = 0; !_in->eof() && !_in->fail() && p < size; p++) {
    result += _in->get();
  }

  size_t zero_byte = result.find('\0');
  return result.substr(0, zero_byte);
}

////////////////////////////////////////////////////////////////////
//     Function: StreamReader::skip_bytes
//       Access: Published
//  Description: Skips over the indicated number of bytes in the
//               stream.
////////////////////////////////////////////////////////////////////
void StreamReader::
skip_bytes(size_t size) {
  nassertv(!_in->eof() && !_in->fail());
  nassertv((int)size >= 0);

  while (size > 0) {
    _in->get();
    size--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StreamReader::extract_bytes
//       Access: Published
//  Description: Extracts the indicated number of bytes in the
//               stream and returns them as a string.
////////////////////////////////////////////////////////////////////
string StreamReader::
extract_bytes(size_t size) {
  nassertr(!_in->eof() && !_in->fail(), string());

  string result;
  result.reserve(size);
  for (size_t p = 0; !_in->eof() && !_in->fail() && p < size; p++) {
    result += _in->get();
  }

  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: StreamReader::readline
//       Access: Published
//  Description: Assumes the stream represents a text file, and
//               extracts one line up to and including the trailing
//               newline character.  Returns empty string when the end
//               of file is reached.
//
//               The interface here is intentionally designed to be
//               similar to that for Python's File.readline()
//               function.
////////////////////////////////////////////////////////////////////
string StreamReader::
readline() {
  string line;
  int ch = _in->get();
  while (!_in->eof() && !_in->fail()) {
    line += ch;
    if (ch == '\n') {
      // Here's the newline character.
      return line;
    }
    ch = _in->get();
  }

  return line;
}

