// Filename: lineStreamBuf.cxx
// Created by:  drose (26Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "lineStreamBuf.h"

#ifndef HAVE_STREAMSIZE
// Some compilers--notably SGI--don't define this for us.
typedef int streamsize;
#endif

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

  pbump(-n);  // Reset pptr().
  return 0;
}
