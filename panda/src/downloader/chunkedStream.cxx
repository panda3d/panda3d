/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file chunkedStream.cxx
 * @author drose
 * @date 2002-09-25
 */

#include "chunkedStream.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

/**
 *
 */
IChunkedStream::
~IChunkedStream() {
  if (_channel != nullptr) {
    _channel->body_stream_destructs(this);
    _channel = nullptr;
  }
}

/**
 * Returns true if the last eof condition was triggered because the socket has
 * genuinely closed, or false if we can expect more data to come along
 * shortly.
 */
bool IChunkedStream::
is_closed() {
  if (_buf._done || _buf.is_closed()) {
    return true;
  }
  clear();
  return false;
}

/**
 * Resets the ChunkedStream to empty, but does not actually close the source
 * BIO unless owns_source was true.
 */
void IChunkedStream::
close() {
  _buf.close_read();
}

/**
 * Returns an enum indicating how we are coming along in reading the document.
 */
IChunkedStream::ReadState IChunkedStream::
get_read_state() {
  return _buf.get_read_state();
}

#endif  // HAVE_OPENSSL
