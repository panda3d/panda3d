// Filename: zStream.h
// Created by:  drose (05Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
public:
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
public:
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


