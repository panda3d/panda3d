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

#ifdef WIN32_VC
  // In spite of the claims of the MSDN Library to the contrary,
  // Windows doesn't seem to provide an allocate() function, so we'll
  // do it by hand.
  char *buf = new char[4096];
  char *ebuf = buf + 4096;
  setg(buf, ebuf, ebuf);
  setp(buf, ebuf, ebuf);

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

  int result = inflateInit(&_z_source);
  if (result < 0) {
    express_cat.warning()
      << "zlib error " << result << " = " << _z_source.msg << "\n";
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
      express_cat.warning()
        << "zlib error " << result << " = " << _z_source.msg << "\n";
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

  int result = deflateInit(&_z_dest, compression_level);
  if (result < 0) {
    express_cat.warning()
      << "zlib error " << result << " = " << _z_dest.msg << "\n";
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
      express_cat.warning()
        << "zlib error " << result << " = " << _z_dest.msg << "\n";
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
  static const size_t decompress_buffer_size = 4096;
  char decompress_buffer[decompress_buffer_size];

  _z_source.next_out = (Bytef *)start;
  _z_source.avail_out = length;

  int flush = (_source->eof() || _source->fail()) ? Z_FINISH : 0;

  while (_z_source.avail_out > 0) {
    if (_z_source.avail_in == 0 && flush == 0) {
      _source->read(decompress_buffer, decompress_buffer_size);
      size_t read_count = _source->gcount();
      if (read_count == 0 || _source->eof() || _source->fail()) {
        // End of input; tell zlib to expect to stop.
        flush = Z_FINISH;
      }
        
      _z_source.next_in = (Bytef *)decompress_buffer;
      _z_source.avail_in = read_count;
    }
    int result = inflate(&_z_source, flush);
    size_t bytes_read = length - _z_source.avail_out;

    if (result == Z_STREAM_END) {
      // Here's the end of the file.
      return bytes_read;
    }
    if (result < 0) {
      express_cat.warning()
        << "zlib error " << result << " = " << _z_source.msg << "\n";
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
    express_cat.warning()
      << "zlib error " << result << " = " << _z_dest.msg << "\n";
  }

  while (_z_dest.avail_in != 0) {
    if (_z_dest.avail_out != compress_buffer_size) {
      _dest->write(compress_buffer, compress_buffer_size - _z_dest.avail_out);
      _z_dest.next_out = (Bytef *)compress_buffer;
      _z_dest.avail_out = compress_buffer_size;
    }
    result = deflate(&_z_dest, flush);
    if (result < 0) {
      express_cat.warning()
        << "zlib error " << result << " = " << _z_dest.msg << "\n";
    }
  }

  while (_z_dest.avail_out != compress_buffer_size) {
    _dest->write(compress_buffer, compress_buffer_size - _z_dest.avail_out);
    _z_dest.next_out = (Bytef *)compress_buffer;
    _z_dest.avail_out = compress_buffer_size;
    result = deflate(&_z_dest, flush);
    if (result < 0 && result != Z_BUF_ERROR) {
      express_cat.warning()
        << "zlib error " << result << " = " << _z_dest.msg << "\n";
    }
  }
}

#endif  // HAVE_ZLIB
