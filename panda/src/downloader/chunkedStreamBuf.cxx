/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file chunkedStreamBuf.cxx
 * @author drose
 * @date 2002-09-25
 */

#include "chunkedStreamBuf.h"
#include "config_downloader.h"
#include <ctype.h>

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

/**
 *
 */
ChunkedStreamBuf::
ChunkedStreamBuf() {
  _chunk_remaining = 0;
  _done = true;
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
ChunkedStreamBuf::
~ChunkedStreamBuf() {
  close_read();
#ifdef PHAVE_IOSTREAM
  PANDA_FREE_ARRAY(_buffer);
#endif
}

/**
 * If the document pointer is non-NULL, it will be updated with the length of
 * the file as it is derived from the chunked encoding.
 */
void ChunkedStreamBuf::
open_read(BioStreamPtr *source, HTTPChannel *doc) {
  _source = source;
  nassertv(!_source.is_null());
  _chunk_remaining = 0;
  _done = false;
  _wanted_nonblocking = doc->_wanted_nonblocking;
  _read_state = ISocketStream::RS_reading;
  _doc = doc;

  if (_doc != nullptr) {
    _read_index = doc->_read_index;
    _doc->_transfer_file_size = 0;
    _doc->_got_transfer_file_size = true;

    // Read a little bit from the file to get the first chunk (and therefore
    // the file size, or at least the size of the first chunk).
    underflow();
  }
}

/**
 *
 */
void ChunkedStreamBuf::
close_read() {
  _source.clear();
}

/**
 * Called by the system istream implementation when its internal buffer needs
 * more characters.
 */
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
size_t ChunkedStreamBuf::
read_chars(char *start, size_t length) {
  while (true) {
    nassertr(!_source.is_null(), 0);
    if (_done) {
      return 0;
    }

    if (_chunk_remaining != 0) {
      // Extract some of the bytes remaining in the chunk.
      length = std::min(length, _chunk_remaining);
      (*_source)->read(start, length);
      size_t read_count = (*_source)->gcount();
      if (!_wanted_nonblocking) {
        while (read_count == 0 && !(*_source)->is_closed()) {
          // Simulate blocking.
          thread_yield();
          (*_source)->read(start, length);
          read_count = (*_source)->gcount();
        }
      }
      _chunk_remaining -= read_count;

      if (read_count == 0 && (*_source)->is_closed()) {
        // Whoops, the socket closed while we were downloading.
        _read_state = ISocketStream::RS_error;
      }

      return read_count;
    }

    // Read the next chunk.
    std::string line;
    bool got_line = http_getline(line);
    while (got_line && line.empty()) {
      // Skip blank lines.  There really should be exactly one blank line, but
      // who's counting?  It's tricky to count and maintain reentry for
      // nonblocking IO.
      got_line = http_getline(line);
    }
    if (!got_line) {
      // EOF (or data unavailable) while trying to read the chunk size.
      if ((*_source)->is_closed()) {
        // Whoops, the socket closed while we were downloading.
        _read_state = ISocketStream::RS_error;
      }

      if (!_wanted_nonblocking) {
        // Simulate blocking.
        thread_yield();
        continue;  // back to the top.
      }

      return 0;
    }
    size_t chunk_size = (size_t)strtol(line.c_str(), nullptr, 16);
    if (downloader_cat.is_spam()) {
      downloader_cat.spam()
        << "Got chunk of size " << chunk_size << " bytes.\n";
    }

    if (chunk_size == 0) {
      // Last chunk; we're done.
      _done = true;
      if (_doc != nullptr && _read_index == _doc->_read_index) {
        _doc->_file_size = _doc->_transfer_file_size;
        _doc->_got_file_size = true;
      }
      _read_state = ISocketStream::RS_complete;
      return 0;
    }

    if (_doc != nullptr && _read_index == _doc->_read_index) {
      _doc->_transfer_file_size += chunk_size;
    }

    _chunk_remaining = chunk_size;

    // Back to the top.
  }

  // Never gets here.
  return 0;
}

/**
 * Reads a single line from the stream.  Returns true if the line is
 * successfully retrieved, or false if a complete line has not yet been
 * received or if the connection has been closed.
 */
bool ChunkedStreamBuf::
http_getline(std::string &str) {
  nassertr(!_source.is_null(), false);
  int ch = (*_source)->get();
  while (!(*_source)->eof() && !(*_source)->fail()) {
    switch (ch) {
    case '\n':
      // end-of-line character, we're done.
      str = _working_getline;
      _working_getline = std::string();
      {
        // Trim trailing whitespace.  We're not required to do this per the
        // HTTP spec, but let's be generous.
        size_t p = str.length();
        while (p > 0 && isspace(str[p - 1])) {
          --p;
        }
        str = str.substr(0, p);
      }

      return true;

    case '\r':
      // Ignore CR characters.
      break;

    default:
      _working_getline += (char)ch;
    }
    ch = (*_source)->get();
  }

  return false;
}

#endif  // HAVE_OPENSSL
