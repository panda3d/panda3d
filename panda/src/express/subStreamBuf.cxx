// Filename: subStreamBuf.cxx
// Created by:  drose (02Aug02)
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

#include "subStreamBuf.h"

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SubStreamBuf::
SubStreamBuf() {
  _source = (istream *)NULL;
  _start = 0;
  _end = 0;
  _cur = 0;
#ifndef WIN32_VC
  // These lines, which are essential on Irix and Linux, seem to be
  // unnecessary and not understood on Windows.
  allocate();
  setg(base(), ebuf(), ebuf());
#endif /* WIN32_VC */
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
SubStreamBuf::
~SubStreamBuf() {
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::open
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SubStreamBuf::
open(istream *source, streampos start, streampos end) {
  _source = source;
  _start = start;
  _end = end;
  _cur = _start;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::close
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SubStreamBuf::
close() {
  sync();
  _source = (istream *)NULL;
  _start = 0;
  _end = 0;
  _cur = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::seekoff
//       Access: Public, Virtual
//  Description: Implements seeking within the stream.
////////////////////////////////////////////////////////////////////
streampos SubStreamBuf::
seekoff(streamoff off, ios::seek_dir dir, int mode) {
  switch (dir) {
  case ios::beg:
    _cur = _start + off;
    break;

  case ios::cur:
    _cur += off;
    break;

  case ios::end:
    _cur = _end + off;
    break;
  }

  _cur = max(_start, _cur);
  return _cur - _start;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::overflow
//       Access: Protected, Virtual
//  Description: Called by the system ostream implementation when its
//               internal buffer is filled, plus one character.
////////////////////////////////////////////////////////////////////
int SubStreamBuf::
SubStreamBuf::overflow(int c) {
  // We don't support ostream.

  /*
  streamsize n = pptr() - pbase();
  if (n != 0) {
    write_chars(pbase(), n, false);
    pbump(-n);  // reset pptr()
  }
  if (c != EOF) {
    // write one more character
    char ch = c;
    write_chars(&ch, 1, false);
  }
  */
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::sync
//       Access: Protected, Virtual
//  Description: Called by the system iostream implementation to
//               implement a flush operation.
////////////////////////////////////////////////////////////////////
int SubStreamBuf::
sync() {
  /*
  streamsize n = pptr() - pbase();
  if (n != 0) {
    write_chars(pbase(), n, false);
    pbump(-n);
  }
  */

  streamsize n = egptr() - gptr();
  if (n != 0) {
    gbump(n);
    _cur += n;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int SubStreamBuf::
underflow() {
  if ((eback() == (char *)NULL) || (gptr() == (char *)NULL) ||
      (egptr() == (char *)NULL)) {
    // No buffer; allocate a new one.  Rumor has it this is only
    // possible in Win32.
    char *buf = new char[4096];
    char *ebuf = buf + 4096;
    setg(buf, ebuf, ebuf);
  }

  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    if (_cur >= _end) {
      // We're done.
      return EOF;
    }
    
    size_t buffer_size = egptr() - eback();
    size_t num_bytes;
    if (_end - _cur > buffer_size) {
      // We have enough bytes in the input stream to fill our buffer.
      num_bytes = buffer_size;
    } else {
      // We won't quite fill the buffer.
      num_bytes = (size_t)(_end - _cur);
    }

    _source->seekg(_cur);
    gbump(-(int)num_bytes);
    nassertr(gptr() + num_bytes <= egptr(), EOF);

    _source->read(gptr(), num_bytes);
    if (_source->gcount() != num_bytes) {
      // Oops, something screwed up.
      _cur = _end;
      return EOF;
    }

    _cur += num_bytes;
  }

  return *gptr();
}
