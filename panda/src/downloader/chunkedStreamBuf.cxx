// Filename: chunkedStreamBuf.cxx
// Created by:  drose (25Sep02)
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

#include "chunkedStreamBuf.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_SSL

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ChunkedStreamBuf::
ChunkedStreamBuf() {
  _chunk_remaining = 0;
  _done = true;

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
//     Function: ChunkedStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ChunkedStreamBuf::
~ChunkedStreamBuf() {
  close_read();
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::open_read
//       Access: Public
//  Description: If the document pointer is non-NULL, it will be
//               updated with the length of the file as it is derived
//               from the chunked encoding.
////////////////////////////////////////////////////////////////////
void ChunkedStreamBuf::
open_read(BioStreamPtr *source, HTTPDocument *doc) {
  _source = source;
  _chunk_remaining = 0;
  _done = false;
  _doc = doc;

  if (_doc != (HTTPDocument *)NULL) {
    _read_index = doc->_read_index;
    _doc->_file_size = 0;

    // Read a little bit from the file to get the first chunk (and
    // therefore the file size, or at least the size of the first
    // chunk).
    underflow();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::close_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ChunkedStreamBuf::
close_read() {
  _source.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ChunkedStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int ChunkedStreamBuf::
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
//     Function: ChunkedStreamBuf::read_chars
//       Access: Private
//  Description: Gets some characters from the source stream.
////////////////////////////////////////////////////////////////////
size_t ChunkedStreamBuf::
read_chars(char *start, size_t length) {
  if (_done) {
    return 0;
  }

  if (_chunk_remaining != 0) {
    // Extract some of the bytes remaining in the chunk.
    length = min(length, _chunk_remaining);
    (*_source)->read(start, length);
    length = (*_source)->gcount();
    _chunk_remaining -= length;
    return length;
  }

  // Read the next chunk.
  string line;
  getline(**_source, line);
  if (!line.empty() && line[line.length() - 1] == '\r') {
    line = line.substr(0, line.length() - 1);
  }
  size_t chunk_size = (size_t)strtol(line.c_str(), NULL, 16);
  if (chunk_size == 0) {
    // Last chunk; we're done.
    _done = true;
    if (_doc != (HTTPDocument *)NULL && _read_index == _doc->_read_index) {
      _doc->_state = HTTPDocument::S_read_body;
    }
    return 0;
  }

  if (_doc != (HTTPDocument *)NULL && _read_index == _doc->_read_index) {
    _doc->_file_size += chunk_size;
  }

  _chunk_remaining = chunk_size;
  return read_chars(start, length);
}

#endif  // HAVE_SSL
