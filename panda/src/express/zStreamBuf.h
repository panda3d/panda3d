// Filename: zStreamBuf.h
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

#ifndef ZSTREAMBUF_H
#define ZSTREAMBUF_H

#include "pandabase.h"

// This module is not compiled if zlib is not available.
#ifdef HAVE_ZLIB

#include <zlib.h>

////////////////////////////////////////////////////////////////////
//       Class : ZStreamBuf
// Description : The streambuf object that implements
//               IDecompressStream and OCompressStream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ZStreamBuf : public streambuf {
public:
  ZStreamBuf();
  virtual ~ZStreamBuf();

  void open_read(istream *source, bool owns_source);
  void close_read();

  void open_write(ostream *dest, bool owns_dest, int compression_level);
  void close_write();

protected:
  virtual int overflow(int c);
  virtual int sync(void);
  virtual int underflow(void);

private:
  size_t read_chars(char *start, size_t length);
  void write_chars(const char *start, size_t length, int flush);
  void show_zlib_error(const char *function, int error_code, z_stream &z);

private:
  istream *_source;
  bool _owns_source;

  ostream *_dest;
  bool _owns_dest;

  z_stream _z_source;
  z_stream _z_dest;
};

#endif  // HAVE_ZLIB

#endif
