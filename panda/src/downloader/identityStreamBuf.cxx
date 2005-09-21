// Filename: identityStreamBuf.cxx
// Created by:  drose (09Oct02)
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

#include "identityStreamBuf.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

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
  _has_content_length = true;
  _bytes_remaining = 0;

#ifdef HAVE_IOSTREAM
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
open_read(BioStreamPtr *source, HTTPChannel *doc, 
          bool has_content_length, size_t content_length) {
  _source = source;
  _doc = doc;
  _has_content_length = has_content_length;
  _bytes_remaining = content_length;

  if (_doc != (HTTPChannel *)NULL) {
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
  _source.clear();
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
        gbump(num_bytes);
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
  size_t read_count = 0;

  if (!_has_content_length) {
    // If we have no restrictions on content length, read till end of
    // file.
    (*_source)->read(start, length);
    read_count = (*_source)->gcount();
  
    if (read_count == 0) {
      if ((*_source)->is_closed()) {
        // socket closed; we're done.
        if (_doc != (HTTPChannel *)NULL && _read_index == _doc->_read_index) {
          _doc->finished_body(false);
        }
      }
      return 0;
    }

  } else {
    // Extract some of the bytes remaining in the chunk.

    if (_bytes_remaining != 0) {
      length = min(length, _bytes_remaining);
      (*_source)->read(start, length);
      read_count = (*_source)->gcount();
      _bytes_remaining -= read_count;
  
      if (read_count == 0) {
        if ((*_source)->is_closed()) {
          // socket closed unexpectedly; problem.
          if (_doc != (HTTPChannel *)NULL && _read_index == _doc->_read_index) {
            _doc->_state = HTTPChannel::S_failure;
          }
        }
        return 0;
      }
    }
      
    if (_bytes_remaining == 0) {
      // We're done.
      if (_doc != (HTTPChannel *)NULL && _read_index == _doc->_read_index) {
        _doc->finished_body(false);
      }
    }
  }

  return read_count;
}

#endif  // HAVE_OPENSSL
