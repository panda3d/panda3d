/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file StreamBufLz4.cxx
 * @author drose
 * @date 2002-08-05
 */

#include <string.h>

#include "streamBufLz4.h"


#include "pnotify.h"
#include "config_express.h"

using std::ios;
using std::streamoff;
using std::streampos;

#if !defined(USE_MEMORY_NOWRAPPERS) && !defined(CPPPARSER)
// Define functions that hook zlib into panda's memory allocation system.
static void *
do_lz4_alloc(voidpf opaque, U64 items, U64 size) {
  return PANDA_MALLOC_ARRAY(items * size);
}
static void
do_lz4_free(voidpf opaque, voidpf address) {
  PANDA_FREE_ARRAY(address);
}
#endif  //  !USE_MEMORY_NOWRAPPERS

/**
 *
 */
StreamBufLz4::
StreamBufLz4() : StreamBufBase() {
    memset(&_lz4_preferences, 0, sizeof(_lz4_preferences));
}

/**
 *
 */
StreamBufLz4::
~StreamBufLz4() {
}

/**
 *
 */
void StreamBufLz4::
open_read(std::istream *source, bool owns_source) {
  StreamBufBase::open_read(source, owns_source);

  size_t result = LZ4F_createDecompressionContext(&_lz4_decompression_ctx, LZ4F_getVersion());
  if (LZ4F_isError(result)) {
      show_lz4_error("LZ4F_createDecompressionContext", result);
      close_read();
  }
  thread_consider_yield();
}

/**
 *
 */
void StreamBufLz4::
close_read() {
  if (_source != nullptr) {

    size_t result = LZ4F_freeDecompressionContext(_lz4_decompression_ctx);
    if (LZ4F_isError(result)) {
      show_lz4_error("LZ4F_freeDecompressionContext", result);
    }
    thread_consider_yield();

    if (_owns_source) {
      delete _source;
      _owns_source = false;
    }
    _source = nullptr;
  }
}

/**
 *
 */
void StreamBufLz4::
open_write(std::ostream *dest, bool owns_dest, int compression_level) {
  StreamBufBase::open_write(dest, owns_dest);

  size_t result = LZ4F_createCompressionContext(&_lz4_compression_ctx, LZ4F_getVersion());
  if (LZ4F_isError(result)) {
    show_lz4_error("LZ4F_createCompressionContext", result);
    close_write();
  }

  _lz4_preferences.compressionLevel = compression_level;
  thread_consider_yield();
}

/**
 *
 */
void StreamBufLz4::
close_write() {
  if (_dest != nullptr) {
    size_t n = pptr() - pbase();

    // autoFlush = 1: always flush
    write_chars(pbase(), n, 1);
    pbump(-(int)n);

    size_t result = LZ4F_freeCompressionContext(_lz4_compression_ctx);
    if (LZ4F_isError(result)) {
      show_lz4_error("LZ4F_freeCompressionContext", result);
    }
    thread_consider_yield();

    if (_owns_dest) {
      delete _dest;
      _owns_dest = false;
    }
    _dest = nullptr;
  }
}

/**
 * Implements seeking within the stream.  StreamBufLz4 only allows seeking back
 * to the beginning of the stream.
 */
streampos StreamBufLz4::
seekoff(streamoff off, ios_seekdir dir, ios_openmode which) {
  if (which != ios::in) {
    // We can only do this with the input stream.
    return -1;
  }

  // Determine the current position.
  size_t n = egptr() - gptr();
  streampos gpos = _lz4_compression_ctx.total_out - n;

  // Implement tellg() and seeks to current position.
  if ((dir == ios::cur && off == 0) ||
      (dir == ios::beg && off == gpos)) {
    return gpos;
  }

  if (off != 0 || dir != ios::beg) {
    // We only know how to reposition to the beginning.
    return -1;
  }

  gbump(n);

  if (_source->rdbuf()->pubseekpos(0, ios::in) == (streampos)0) {
    _source->clear();
    _lz4_decompression_ctx.tmpIn = nullptr;
    _lz4_decompression_ctx.tmpInSize = 0;
    LZ4F_resetDecompressionContext(_lz4_decompression_ctx);
    return 0;
  }

  return -1;
}

/**
 * Implements seeking within the stream.  StreamBufLz4 only allows seeking back
 * to the beginning of the stream.
 */
streampos StreamBufLz4::
seekpos(streampos pos, ios_openmode which) {
  return seekoff(pos, ios::beg, which);
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int StreamBufLz4::
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
int StreamBufLz4::
sync() {
  if (_source != nullptr) {
    size_t n = egptr() - gptr();
    gbump(n);
  }

  if (_dest != nullptr) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n, 1);
    pbump(-(int)n);
  }

  _dest->flush();
  return 0;
}

/**
 * Called by the system istream implementation when its internal buffer needs
 * more characters.
 */
int StreamBufLz4::
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
size_t StreamBufLz4::
read_chars(char *start, size_t length) {
  bool eof = (_source->eof() || _source->fail());
  int flush = 0;

  size_t remaining_length = length;
  size_t dst_size = 0;
  size_t *dst_size_ptr = &dst_size;
  size_t src_size = 0;
  size_t *src_size_ptr = &src_size;

  while (remaining_length > 0) {
    if (*dst_size_ptr != 0 && !eof) {
      _source->read(decompress_buffer, *dst_size_ptr);
      *dst_size_ptr = decompress_buffer_size;
    }
    remaining_length -= *src_size_ptr;
    *src_size_ptr = remaining_length;
    size_t result = LZ4F_decompress(_lz4_decompression_ctx, &decompress_buffer, dst_size_ptr,
                                    start + (length - remaining_length), src_size_ptr, nullptr);
    thread_consider_yield();

    if (LZ4F_isError(result)) {
      show_lz4_error("LZ4F_decompress", result);
      return length - remaining_length;
    }
  }

  return length;
}

/**
 * Sends some characters to the dest stream.  The flush parameter is passed to
 * deflate().
 */
void StreamBufLz4::
write_chars(const char *start, size_t length, int flush) {
  static const size_t compress_buffer_size = 4096;
  char compress_buffer[compress_buffer_size];
  const char *current = start;

  while(true)
  {
      size_t compressed_length = min(length - (current - start), (size_t)4096);
      size_t result = LZ4F_compressUpdate(_lz4_compression_ctx, compress_buffer, compress_buffer_size,
                                       current, compressed_length, nullptr);
      if (LZ4F_isError(result)) {
        show_lz4_error("LZ4F_compressUpdate", result);
      }
      thread_consider_yield();

      _dest->write(compress_buffer, result);
      current += compressed_length;
  }
}

/**
 * Reports a recent error code returned by zlib.
 */
void StreamBufLz4::
show_lz4_error(const char *function, int error_code) {
  std::stringstream error_line;

  error_line
    << "lz4 error in " << function << ": " << LZ4F_getErrorName(error_code);
  }

  express_cat.warning() << error_line.str() << "\n";
}

//#endif  // HAVE_LZ4
