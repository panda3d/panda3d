// Filename: lineStreamBuf.cxx
// Created by:  drose (26Feb00)
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

#include "lineStreamBuf.h"

#ifndef HAVE_STREAMSIZE
// Some compilers--notably SGI--don't define this for us.
typedef int streamsize;
#endif

////////////////////////////////////////////////////////////////////
//     Function: LineStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LineStreamBuf::
LineStreamBuf() {
  _has_newline = false;

  // The LineStreamBuf doesn't actually need a buffer--it's happy
  // writing characters one at a time, since they're just getting
  // stuffed into a string.  (Although the code is written portably
  // enough to use a buffer correctly, if we had one.)
  setg(0, 0, 0);
  setp(0, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LineStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
LineStreamBuf::
~LineStreamBuf() {
  sync();
}

////////////////////////////////////////////////////////////////////
//     Function: LineStreamBuf::get_line
//       Access: Public
//  Description: Extracts the next line of text from the
//               LineStreamBuf, and sets the has_newline() flag
//               according to whether this line had a trailing newline
//               or not.
////////////////////////////////////////////////////////////////////
string LineStreamBuf::
get_line() {
  // Extract the data up to, but not including, the next newline
  // character.
  size_t nl = _data.find('\n');
  if (nl == string::npos) {
    // No trailing newline; return the remainder of the string.
    _has_newline = false;
    string result = _data;
    _data = "";
    return result;
  }

  _has_newline = true;
  string result = _data.substr(0, nl);
  _data = _data.substr(nl + 1);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LineStreamBuf::sync
//       Access: Public, Virtual
//  Description: Called by the system ostream implementation when the
//               buffer should be flushed to output (for instance, on
//               destruction).
////////////////////////////////////////////////////////////////////
int LineStreamBuf::
sync() {
  streamsize n = pptr() - pbase();
  write_chars(pbase(), n);
  pbump(-n);  // Reset pptr().
  return 0;  // EOF to indicate write full.
}

////////////////////////////////////////////////////////////////////
//     Function: LineStreamBuf::overflow
//       Access: Public, Virtual
//  Description: Called by the system ostream implementation when its
//               internal buffer is filled, plus one character.
////////////////////////////////////////////////////////////////////
int LineStreamBuf::
overflow(int ch) {
  streamsize n = pptr() - pbase();

  if (n != 0 && sync() != 0) {
    return EOF;
  }

  if (ch != EOF) {
    // Write one more character.
    char c = ch;
    write_chars(&c, 1);
  }

  return 0;
}
