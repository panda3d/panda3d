// Filename: lineStreamBuf.I
// Created by:  drose (26Feb00)
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


////////////////////////////////////////////////////////////////////
//     Function: LineStreamBuf::is_text_available
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool LineStreamBuf::
is_text_available() const {
  return !_data.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: LineStreamBuf::has_newline
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool LineStreamBuf::
has_newline() const {
  return _has_newline;
}

////////////////////////////////////////////////////////////////////
//     Function: LineStreamBuf::write_chars
//       Access: Private
//  Description: An internal function called by sync() and overflow()
//               to store one or more characters written to the stream
//               into the memory buffer.
////////////////////////////////////////////////////////////////////
INLINE void LineStreamBuf::
write_chars(const char *start, int length) {
  if (length > 0) {
    _data += string(start, length);
  }
}
