// Filename: zStreamBuf.cxx
// Created by:  drose (05Aug02)
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

#include "zStreamBuf.h"

#ifdef HAVE_ZLIB

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ZStreamBuf::
ZStreamBuf() {
  _source = (istream *)NULL;
  _owns_source = false;
  _dest = (ostream *)NULL;
  _owns_dest = false;

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
//     Function: ZStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ZStreamBuf::
~ZStreamBuf() {
  close_read();
  close_write();
}

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::open_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ZStreamBuf::
open_read(istream *source, bool owns_source) {
  _source = source;
  _owns_source = owns_source;

  _z_source.next_in = Z_NULL;
  _z_source.avail_in = 0;
  _z_source.zalloc = Z_NULL;
  _z_source.zfree = Z_NULL;
  _z_source.opaque = Z_NULL;
  _z_source.msg = "no error message";

  int result = inflateInit(&_z_source);
  if (result < 0) {
    show_zlib_error("inflateInit", result, _z_source);
    close_read();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::close_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ZStreamBuf::
close_read() {
  if (_source != (istream *)NULL) {

    int result = inflateEnd(&_z_source);
    if (result < 0) {
      show_zlib_error("inflateEnd", result, _z_source);
    }

    if (_owns_source) {
      delete _source;
      _owns_source = false;
    }
    _source = (istream *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::open_write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ZStreamBuf::
open_write(ostream *dest, bool owns_dest, int compression_level) {
  _dest = dest;
  _owns_dest = owns_dest;

  _z_dest.zalloc = Z_NULL;
  _z_dest.zfree = Z_NULL;
  _z_dest.opaque = Z_NULL;
  _z_dest.msg = "no error message";

  int result = deflateInit(&_z_dest, compression_level);
  if (result < 0) {
    show_zlib_error("deflateInit", result, _z_dest);
    close_write();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::close_write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ZStreamBuf::
close_write() {
  if (_dest != (ostream *)NULL) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n, Z_FINISH);
    pbump(-(int)n);

    int result = deflateEnd(&_z_dest);
    if (result < 0) {
      show_zlib_error("deflateEnd", result, _z_dest);
    }

    if (_owns_dest) {
      delete _dest;
      _owns_dest = false;
    }
    _dest = (ostream *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::overflow
//       Access: Protected, Virtual
//  Description: Called by the system ostream implementation when its
//               internal buffer is filled, plus one character.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::sync
//       Access: Protected, Virtual
//  Description: Called by the system iostream implementation to
//               implement a flush operation.
////////////////////////////////////////////////////////////////////
int ZStreamBuf::
sync() {
  if (_source != (istream *)NULL) {
    size_t n = egptr() - gptr();
    gbump(n);
  }

  if (_dest != (ostream *)NULL) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n, Z_SYNC_FLUSH);
    pbump(-(int)n);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::read_chars
//       Access: Private
//  Description: Gets some characters from the source stream.
////////////////////////////////////////////////////////////////////
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
    size_t bytes_read = length - _z_source.avail_out;

    if (result == Z_STREAM_END) {
      // Here's the end of the file.
      return bytes_read;

    } else if (result == Z_BUF_ERROR && flush == 0) {
      // We might get this if no progress is possible, for instance if
      // the input stream is truncated.  In this case, tell zlib to
      // dump everything it's got.
      flush = Z_FINISH;

    } else if (result < 0) {
      show_zlib_error("inflate", result, _z_source);
      return bytes_read;
    }
  }

  return length;
}

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::write_chars
//       Access: Private
//  Description: Sends some characters to the dest stream.  The flush
//               parameter is passed to deflate().
////////////////////////////////////////////////////////////////////
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
  }

  while (_z_dest.avail_out != compress_buffer_size) {
    _dest->write(compress_buffer, compress_buffer_size - _z_dest.avail_out);
    _z_dest.next_out = (Bytef *)compress_buffer;
    _z_dest.avail_out = compress_buffer_size;
    result = deflate(&_z_dest, flush);
    if (result < 0 && result != Z_BUF_ERROR) {
      show_zlib_error("deflate", result, _z_dest);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ZStreamBuf::show_zlib_error
//       Access: Private
//  Description: Reports a recent error code returned by zlib.
////////////////////////////////////////////////////////////////////
void ZStreamBuf::
show_zlib_error(const char *function, int error_code, z_stream &z) {
  stringstream error_line;

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
  if (z.msg != (char *)NULL) {
    error_line
      << " = " << z.msg;
  }

  express_cat.warning() << error_line.str() << "\n";
}

#endif  // HAVE_ZLIB
