// Filename: identityStreamBuf.cxx
// Created by:  drose (09Oct02)
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

#include "identityStreamBuf.h"

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

////////////////////////////////////////////////////////////////////
//     Function: IdentityStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
IdentityStreamBuf::
IdentityStreamBuf() {
  _source = (istream *)NULL;
  _owns_source = false;
  _bytes_remaining = 0;

#ifdef WIN32_VC
  // In spite of the claims of the MSDN Library to the contrary,
  // Windows doesn't seem to provide an allocate() function, so we'll
  // do it by hand.
  char *buf = new char[4096];
  char *ebuf = buf + 4096;
  setg(buf, ebuf, ebuf);
  setp(buf, ebuf);

#else
  allocate();
  setg(base(), ebuf(), ebuf());
  setp(base(), ebuf());
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: IdentityStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
IdentityStreamBuf::
~IdentityStreamBuf() {
  close_read();
}

////////////////////////////////////////////////////////////////////
//     Function: IdentityStreamBuf::open_read
//       Access: Public
//  Description: If the document pointer is non-NULL, it will be
//               updated with the length of the file as it is derived
//               from the identity encoding.
////////////////////////////////////////////////////////////////////
void IdentityStreamBuf::
open_read(istream *source, bool owns_source, HTTPDocument *doc, 
          size_t content_length) {
  _source = source;
  _owns_source = owns_source;
  _doc = doc;
  _bytes_remaining = content_length;

  if (_doc != (HTTPDocument *)NULL) {
    _read_index = doc->_read_index;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IdentityStreamBuf::close_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void IdentityStreamBuf::
close_read() {
  if (_source != (istream *)NULL) {
    if (_owns_source) {
      delete _source;
      _owns_source = false;
    }
    _source = (istream *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IdentityStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int IdentityStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    size_t buffer_size = egptr() - eback();
    gbump(-(int)buffer_size);

    size_t num_bytes = buffer_size;
    size_t read_count = read_chars(gptr(), buffer_size);

    if (read_count != num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < num_bytes, EOF);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  return (unsigned char)*gptr();
}


////////////////////////////////////////////////////////////////////
//     Function: IdentityStreamBuf::read_chars
//       Access: Private
//  Description: Gets some characters from the source stream.
////////////////////////////////////////////////////////////////////
size_t IdentityStreamBuf::
read_chars(char *start, size_t length) {
  if (_bytes_remaining == 0) {
    return 0;
  }

  // Extract some of the bytes remaining in the chunk.
  length = min(length, _bytes_remaining);
  _source->read(start, length);
  length = _source->gcount();
  _bytes_remaining -= length;

  if (_bytes_remaining == 0) {
    // We're done.
    if (_doc != (HTTPDocument *)NULL && _read_index == _doc->_read_index) {
      // An IdentityStreamBuf doesn't have a trailer, so we've already
      // "read" it.
      _doc->_state = HTTPDocument::S_read_trailer;
    }
  }

  return length;
}
