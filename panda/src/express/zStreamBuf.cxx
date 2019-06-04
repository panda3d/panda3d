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

#include "zStreamBuf.h"

#ifdef HAVE_ZLIB

#include "pnotify.h"
#include "config_express.h"

using std::ios;
using std::streamoff;
using std::streampos;

#if !defined(USE_MEMORY_NOWRAPPERS) && !defined(CPPPARSER)
// Define functions that hook zlib into panda's memory allocation system.
static void *
do_zlib_alloc(voidpf opaque, uInt items, uInt size) {
  return PANDA_MALLOC_ARRAY(items * size);
}
static void
do_zlib_free(voidpf opaque, voidpf address) {
  PANDA_FREE_ARRAY(address);
}
#endif  //  !USE_MEMORY_NOWRAPPERS

/**
 *
 */
ZStreamBuf::
ZStreamBuf() {
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
ZStreamBuf::
~ZStreamBuf() {
  close_read();
  close_write();
#ifdef PHAVE_IOSTREAM
  PANDA_FREE_ARRAY(_buffer);
#endif
}

/**
 *
 */
void ZStreamBuf::
open_read(std::istream *source, bool owns_source) {
  _source = source;
  _owns_source = owns_source;

  _z_source.next_in = Z_NULL;
  _z_source.avail_in = 0;
  _z_source.next_out = Z_NULL;
  _z_source.avail_out = 0;
#ifdef USE_MEMORY_NOWRAPPERS
  _z_source.zalloc = Z_NULL;
  _z_source.zfree = Z_NULL;
#else
  _z_source.zalloc = (alloc_func)&do_zlib_alloc;
  _z_source.zfree = (free_func)&do_zlib_free;
#endif
  _z_source.opaque = Z_NULL;
  _z_source.msg = (char *)"no error message";

  int result = inflateInit2(&_z_source, 32 + 15);
  if (result < 0) {
    show_zlib_error("inflateInit2", result, _z_source);
    close_read();
  }
  thread_consider_yield();
}

/**
 *
 */
void ZStreamBuf::
close_read() {
  if (_source != nullptr) {

    int result = inflateEnd(&_z_source);
    if (result < 0) {
      show_zlib_error("inflateEnd", result, _z_source);
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
void ZStreamBuf::
open_write(std::ostream *dest, bool owns_dest, int compression_level) {
  _dest = dest;
  _owns_dest = owns_dest;

  _z_dest.next_in = Z_NULL;
  _z_dest.avail_in = 0;
  _z_dest.next_out = Z_NULL;
  _z_dest.avail_out = 0;
#ifdef USE_MEMORY_NOWRAPPERS
  _z_dest.zalloc = Z_NULL;
  _z_dest.zfree = Z_NULL;
#else
  _z_dest.zalloc = (alloc_func)&do_zlib_alloc;
  _z_dest.zfree = (free_func)&do_zlib_free;
#endif
  _z_dest.opaque = Z_NULL;
  _z_dest.msg = (char *)"no error message";

  int result = deflateInit(&_z_dest, compression_level);
  if (result < 0) {
    show_zlib_error("deflateInit", result, _z_dest);
    close_write();
  }
  thread_consider_yield();
}

/**
 *
 */
void ZStreamBuf::
close_write() {
  if (_dest != nullptr) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n, Z_FINISH);
    pbump(-(int)n);

    int result = deflateEnd(&_z_dest);
    if (result < 0) {
      show_zlib_error("deflateEnd", result, _z_dest);
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
 * Implements seeking within the stream.  ZStreamBuf only allows seeking back
 * to the beginning of the stream.
 */
streampos ZStreamBuf::
seekoff(streamoff off, ios_seekdir dir, ios_openmode which) {
  if (which != ios::in) {
    // We can only do this with the input stream.
    return -1;
  }

  // Determine the current position.
  size_t n = egptr() - gptr();
  streampos gpos = _z_source.total_out - n;

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
    _z_source.next_in = Z_NULL;
    _z_source.avail_in = 0;
    _z_source.next_out = Z_NULL;
    _z_source.avail_out = 0;
    int result = inflateReset(&_z_source);
    if (result < 0) {
      show_zlib_error("inflateReset", result, _z_source);
    }
    return 0;
  }

  return -1;
}

/**
 * Implements seeking within the stream.  ZStreamBuf only allows seeking back
 * to the beginning of the stream.
 */
streampos ZStreamBuf::
seekpos(streampos pos, ios_openmode which) {
  return seekoff(pos, ios::beg, which);
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int ZStreamBuf::
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
int ZStreamBuf::
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
int ZStreamBuf::
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
size_t ZStreamBuf::
read_chars(char *start, size_t length) {
  _z_source.next_out = (Bytef *)start;
  _z_source.avail_out = length;

  bool eof = (_source->eof() || _source->fail());
  int flush = 0;

  while (_z_source.avail_out > 0) {
    if (_z_source.avail_in == 0 && !eof) {
      _source->read(decompress_buffer, decompress_buffer_size);
      size_t read_count = _source->gcount();
      eof = (read_count == 0 || _source->eof() || _source->fail());

      _z_source.next_in = (Bytef *)decompress_buffer;
      _z_source.avail_in = read_count;
    }
    int result = inflate(&_z_source, flush);
    thread_consider_yield();
    size_t bytes_read = length - _z_source.avail_out;

    if (result == Z_STREAM_END) {
      // Here's the end of the file.
      return bytes_read;

    } else if (result == Z_BUF_ERROR && flush == 0) {
      // We might get this if no progress is possible, for instance if the
      // input stream is truncated.  In this case, tell zlib to dump
      // everything it's got.
      flush = Z_FINISH;

    } else if (result < 0) {
      show_zlib_error("inflate", result, _z_source);
      return bytes_read;
    }
  }

  return length;
}

/**
 * Sends some characters to the dest stream.  The flush parameter is passed to
 * deflate().
 */
void ZStreamBuf::
write_chars(const char *start, size_t length, int flush) {
  static const size_t compress_buffer_size = 4096;
  char compress_buffer[compress_buffer_size];

  _z_dest.next_in = (Bytef *)(char *)start;
  _z_dest.avail_in = length;

  _z_dest.next_out = (Bytef *)compress_buffer;
  _z_dest.avail_out = compress_buffer_size;

  int result = deflate(&_z_dest, flush);
  if (result < 0 && result != Z_BUF_ERROR) {
    show_zlib_error("deflate", result, _z_dest);
  }
  thread_consider_yield();

  while (_z_dest.avail_in != 0) {
    if (_z_dest.avail_out != compress_buffer_size) {
      _dest->write(compress_buffer, compress_buffer_size - _z_dest.avail_out);
      _z_dest.next_out = (Bytef *)compress_buffer;
      _z_dest.avail_out = compress_buffer_size;
    }
    result = deflate(&_z_dest, flush);
    if (result < 0) {
      show_zlib_error("deflate", result, _z_dest);
    }
    thread_consider_yield();
  }

  while (_z_dest.avail_out != compress_buffer_size) {
    _dest->write(compress_buffer, compress_buffer_size - _z_dest.avail_out);
    _z_dest.next_out = (Bytef *)compress_buffer;
    _z_dest.avail_out = compress_buffer_size;
    result = deflate(&_z_dest, flush);
    if (result < 0 && result != Z_BUF_ERROR) {
      show_zlib_error("deflate", result, _z_dest);
    }
    thread_consider_yield();
  }
}

/**
 * Reports a recent error code returned by zlib.
 */
void ZStreamBuf::
show_zlib_error(const char *function, int error_code, z_stream &z) {
  std::stringstream error_line;

  error_line
    << "zlib error in " << function << ": ";
  switch (error_code) {
  case Z_OK:
    error_line << "Z_OK";
    break;
  case Z_STREAM_END:
    error_line << "Z_STREAM_END";
    break;
  case Z_NEED_DICT:
    error_line << "Z_NEED_DICT";
    break;
  case Z_ERRNO:
    error_line << "Z_ERRNO";
    break;
  case Z_STREAM_ERROR:
    error_line << "Z_STREAM_ERROR";
    break;
  case Z_DATA_ERROR:
    error_line << "Z_DATA_ERROR";
    break;
  case Z_MEM_ERROR:
    error_line << "Z_MEM_ERROR";
    break;
  case Z_BUF_ERROR:
    error_line << "Z_BUF_ERROR";
    break;
  case Z_VERSION_ERROR:
    error_line << "Z_VERSION_ERROR";
    break;
  default:
    error_line << error_code;
  }
  if (z.msg != nullptr) {
    error_line
      << " = " << z.msg;
  }

  express_cat.warning() << error_line.str() << "\n";
}

#endif  // HAVE_ZLIB
