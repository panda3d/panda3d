/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zStreamBuf.h
 * @author drose
 * @date 2002-08-05
 */

#ifndef ZSTREAMBUF_H
#define ZSTREAMBUF_H

#include "pandabase.h"

// This module is not compiled if zlib is not available.
#ifdef HAVE_ZLIB

#include <zlib.h>

/**
 * The streambuf object that implements IDecompressStream and OCompressStream.
 */
class EXPCL_PANDA_EXPRESS ZStreamBuf : public std::streambuf {
public:
  ZStreamBuf();
  virtual ~ZStreamBuf();

  void open_read(std::istream *source, bool owns_source);
  void close_read();

  void open_write(std::ostream *dest, bool owns_dest, int compression_level);
  void close_write();

  virtual std::streampos seekoff(std::streamoff off, ios_seekdir dir, ios_openmode which);
  virtual std::streampos seekpos(std::streampos pos, ios_openmode which);

protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  void write_chars(const char *start, size_t length, int flush);
  void show_zlib_error(const char *function, int error_code, z_stream &z);

private:
  std::istream *_source;
  bool _owns_source;

  std::ostream *_dest;
  bool _owns_dest;

  z_stream _z_source;
  z_stream _z_dest;

  char *_buffer;

  // We need to store the decompression buffer on the class object, because
  // zlib might not consume all of the input characters at each call to
  // inflate().  This isn't a problem on output because in that case we can
  // afford to wait until it does consume all of the characters we give it.
  enum {
    // It's not clear how large or small this buffer ought to be.  It doesn't
    // seem to matter much, especially since this is just a temporary holding
    // area before getting copied into zlib's own internal buffers.
    decompress_buffer_size = 128
  };
  char decompress_buffer[decompress_buffer_size];
};

#endif  // HAVE_ZLIB

#endif
