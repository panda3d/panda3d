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

#ifndef STREAMBUF_BASE_H
#define STREAMBUF_BASE_H

#include "pandabase.h"

// This module is not compiled if both zlib and lz4 are not available.
#if defined(HAVE_ZLIB) || defined(HAVE_LZ4)

/**
 * The streambuf object that implements IDecompressStream and OCompressStream.
 */
class EXPCL_PANDA_EXPRESS StreamBufBase : public std::streambuf {
public:
  StreamBufBase();
  virtual ~StreamBufBase();

  void virtual open_read(std::istream *source, bool owns_source);
  void virtual close_read();

  void virtual open_write(std::ostream *dest, bool owns_dest);
  void virtual close_write();

  virtual std::streampos seekoff(std::streamoff off, ios_seekdir dir, ios_openmode which);
  virtual std::streampos seekpos(std::streampos pos, ios_openmode which);

protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  void write_chars(const char *start, size_t length, int flush);

private:
  std::istream *_source;
  bool _owns_source;

  std::ostream *_dest;
  bool _owns_dest;

  char *_buffer;

  // We need to store the decompression buffer on the class object, because
  // compression library might not consume all of the input characters at each call to
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

#endif  // HAVE_ZLIB || HAVE_LZ4

#endif
