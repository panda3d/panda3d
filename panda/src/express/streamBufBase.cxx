/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zStreamBuf.cxx
 * @author drose
 * @date 2002-08-05
 */

#include "streamBufBase.h"

#if defined(HAVE_ZLIB) || defined (HAVE_LZ4)

#include "pnotify.h"
#include "config_express.h"

using std::ios;
using std::streamoff;
using std::streampos;

/**
 *
 */
StreamBufBase::
StreamBufBase() {
  _source = nullptr;
  _owns_source = false;
  _dest = nullptr;
  _owns_dest = false;

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
StreamBufBase::
~StreamBufBase() {
  close_read();
  close_write();
#ifdef PHAVE_IOSTREAM
  PANDA_FREE_ARRAY(_buffer);
#endif
}

/**
 *
 */
void StreamBufBase::
open_read(std::istream *source, bool owns_source) {
  _source = source;
  _owns_source = owns_source;
}

/**
 *
 */
void StreamBufBase::
close_read() {
}

/**
 *
 */
void StreamBufBase::
open_write(std::ostream *dest, bool owns_dest) {
  _dest = dest;
  _owns_dest = owns_dest;
}

/**
 *
 */
void StreamBufBase::
close_write() {
}

/**
 * Implements seeking within the stream.  ZStreamBuf only allows seeking back
 * to the beginning of the stream.
 */
streampos StreamBufBase::
seekoff(streamoff off, ios_seekdir dir, ios_openmode which) {
  if (which != ios::in) {
    // We can only do this with the input stream.
    return -1;
  }
}

/**
 * Implements seeking within the stream.  ZStreamBuf only allows seeking back
 * to the beginning of the stream.
 */
streampos StreamBufBase::
seekpos(streampos pos, ios_openmode which) {
  return seekoff(pos, ios::beg, which);
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int StreamBufBase::
overflow(int ch) {
  size_t n = pptr() - pbase();
  if (n != 0) {
    write_chars(pbase(), n, 0);
    pbump(-(int)n);
  }

  if (ch != EOF) {
    // Write one more character.
    char c = ch;
    write_chars(&c, 1, 0);
  }

  return 0;
}

/**
 * Called by the system iostream implementation to implement a flush
 * operation.
 */
int StreamBufBase::
sync() {
  if (_source != nullptr) {
    size_t n = egptr() - gptr();
    gbump(n);
  }

  if (_dest != nullptr) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n, Z_SYNC_FLUSH);
    pbump(-(int)n);
  }

  _dest->flush();
  return 0;
}

/**
 * Called by the system istream implementation when its internal buffer needs
 * more characters.
 */
int StreamBufBase::
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
size_t StreamBufBase::
read_chars(char *start, size_t length) {
}

/**
 * Sends some characters to the dest stream.  The flush parameter is passed to
 * deflate().
 */
void StreamBufBase::
write_chars(const char *start, size_t length, int flush) {
  static const size_t compress_buffer_size = 4096;
  char compress_buffer[compress_buffer_size];
}

#endif  // HAVE_ZLIB
