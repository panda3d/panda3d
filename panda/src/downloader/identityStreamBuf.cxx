/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file identityStreamBuf.cxx
 * @author drose
 * @date 2002-10-09
 */

#include "identityStreamBuf.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL
#include "httpChannel.h"

/**
 *
 */
IdentityStreamBuf::
IdentityStreamBuf() {
  _has_content_length = true;
  _bytes_remaining = 0;
  _wanted_nonblocking = false;
  _read_state = ISocketStream::RS_initial;

#ifdef PHAVE_IOSTREAM
  _buffer = (char *)PANDA_MALLOC_ARRAY(4096);
  char *ebuf = _buffer + 4096;
  setg(_buffer, ebuf, ebuf);
  setp(_buffer, ebuf);

#else
  allocate();
  setg(base(), ebuf(), ebuf());
  setp(base(), ebuf());
#endif
}

/**
 *
 */
IdentityStreamBuf::
~IdentityStreamBuf() {
  close_read();
#ifdef PHAVE_IOSTREAM
  PANDA_FREE_ARRAY(_buffer);
#endif
}

/**
 * If the document pointer is non-NULL, it will be updated with the length of
 * the file as it is derived from the identity encoding.
 */
void IdentityStreamBuf::
open_read(BioStreamPtr *source, HTTPChannel *doc,
          bool has_content_length, size_t content_length) {
  _source = source;
  _has_content_length = has_content_length;
  _wanted_nonblocking = doc->_wanted_nonblocking;
  _bytes_remaining = content_length;
  _read_state = ISocketStream::RS_reading;
}

/**
 *
 */
void IdentityStreamBuf::
close_read() {
  _source.clear();
}

/**
 * Called by the system istream implementation when its internal buffer needs
 * more characters.
 */
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


/**
 * Gets some characters from the source stream.
 */
size_t IdentityStreamBuf::
read_chars(char *start, size_t length) {
  size_t read_count = 0;

  if (!_has_content_length) {
    // If we have no restrictions on content length, read till end of file.
    (*_source)->read(start, length);
    read_count = (*_source)->gcount();

    if (!_wanted_nonblocking) {
      while (read_count == 0 && !(*_source)->is_closed()) {
        // Simulate blocking.
        thread_yield();
        (*_source)->read(start, length);
        read_count = (*_source)->gcount();
      }
    }

    if (read_count == 0) {
      if ((*_source)->is_closed()) {
        // socket closed; we're done.
        _read_state = ISocketStream::RS_complete;
      }
      return 0;
    }

  } else {
    // Extract some of the remaining bytes, but do not read past the
    // content_length restriction.

    if (_bytes_remaining != 0) {
      length = std::min(length, _bytes_remaining);
      (*_source)->read(start, length);
      read_count = (*_source)->gcount();
      if (!_wanted_nonblocking) {
        while (read_count == 0 && !(*_source)->is_closed()) {
          // Simulate blocking.
          thread_yield();
          (*_source)->read(start, length);
          read_count = (*_source)->gcount();
        }
      }
      nassertr(read_count <= _bytes_remaining, 0);
      _bytes_remaining -= read_count;

      if (read_count == 0) {
        if ((*_source)->is_closed()) {
          // socket closed unexpectedly; problem.
          _read_state = ISocketStream::RS_error;
        }
        return 0;
      }
    }

    if (_bytes_remaining == 0) {
      // We're done.
      _read_state = ISocketStream::RS_complete;
    }
  }

  return read_count;
}

#endif  // HAVE_OPENSSL
