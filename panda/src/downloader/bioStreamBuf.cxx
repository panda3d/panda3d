// Filename: bioStreamBuf.cxx
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

#include "bioStreamBuf.h"

#ifdef HAVE_SSL

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

////////////////////////////////////////////////////////////////////
//     Function: BioStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BioStreamBuf::
BioStreamBuf() {
  _is_closed = false;

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
//     Function: BioStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BioStreamBuf::
~BioStreamBuf() {
  close_read();
}

////////////////////////////////////////////////////////////////////
//     Function: BioStreamBuf::open_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BioStreamBuf::
open_read(BioPtr *source) {
  _source = source;
}

////////////////////////////////////////////////////////////////////
//     Function: BioStreamBuf::close_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BioStreamBuf::
close_read() {
  _source.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: BioStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int BioStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    size_t buffer_size = egptr() - eback();
    gbump(-(int)buffer_size);

    size_t num_bytes = buffer_size;

    // BIO_read might return -1 or -2 on eof or error, so we have to
    // allow for negative numbers.
    int read_count = BIO_read(*_source, gptr(), buffer_size);

    if (read_count != (int)num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count <= 0) {
        _is_closed = !BIO_should_retry(*_source);
        gbump(num_bytes);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < (int)num_bytes, EOF);
      size_t delta = (int)num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  return (unsigned char)*gptr();
}


#endif  // HAVE_SSL
