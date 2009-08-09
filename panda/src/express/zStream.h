// Filename: zStream.h
// Created by:  drose (05Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef ZSTREAM_H
#define ZSTREAM_H

#include "pandabase.h"

// This module is not compiled if zlib is not available.
#ifdef HAVE_ZLIB

#include "zStreamBuf.h"

////////////////////////////////////////////////////////////////////
//       Class : IDecompressStream
// Description : An input stream object that uses zlib to decompress
//               (inflate) the input from another source stream
//               on-the-fly.
//
//               Attach an IDecompressStream to an existing istream that
//               provides compressed data, and read the corresponding
//               uncompressed data from the IDecompressStream.
//
//               Seeking is not supported.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS IDecompressStream : public istream {
PUBLISHED:
  INLINE IDecompressStream();
  INLINE IDecompressStream(istream *source, bool owns_source);

  INLINE IDecompressStream &open(istream *source, bool owns_source);
  INLINE IDecompressStream &close();

private:
  ZStreamBuf _buf;
};

////////////////////////////////////////////////////////////////////
//       Class : OCompressStream
// Description : An input stream object that uses zlib to compress
//               (deflate) data to another destination stream
//               on-the-fly.
//
//               Attach an OCompressStream to an existing ostream that will
//               accept compressed data, and write your uncompressed
//               source data to the OCompressStream.
//
//               Seeking is not supported.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS OCompressStream : public ostream {
PUBLISHED:
  INLINE OCompressStream();
  INLINE OCompressStream(ostream *dest, bool owns_dest, 
                           int compression_level = 6);

  INLINE OCompressStream &open(ostream *dest, bool owns_dest, 
                               int compression_level = 6);
  INLINE OCompressStream &close();

private:
  ZStreamBuf _buf;
};

#include "zStream.I"

#endif  // HAVE_ZLIB


#endif


