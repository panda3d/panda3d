/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zStream.h
 * @author drose
 * @date 2002-08-05
 */

#ifndef ZSTREAM_H
#define ZSTREAM_H

#include "pandabase.h"

// This module is not compiled if zlib is not available.
#ifdef HAVE_ZLIB

#include "zStreamBuf.h"

/**
 * An input stream object that uses zlib to decompress (inflate) the input
 * from another source stream on-the-fly.
 *
 * Attach an IDecompressStream to an existing istream that provides compressed
 * data, and read the corresponding uncompressed data from the
 * IDecompressStream.
 *
 * Seeking is not supported.
 */
class EXPCL_PANDA_EXPRESS IDecompressStream : public std::istream {
PUBLISHED:
  INLINE IDecompressStream();
  INLINE explicit IDecompressStream(std::istream *source, bool owns_source);

#if _MSC_VER >= 1800
  INLINE IDecompressStream(const IDecompressStream &copy) = delete;
#endif

  INLINE IDecompressStream &open(std::istream *source, bool owns_source);
  INLINE IDecompressStream &close();

private:
  ZStreamBuf _buf;
};

/**
 * An input stream object that uses zlib to compress (deflate) data to another
 * destination stream on-the-fly.
 *
 * Attach an OCompressStream to an existing ostream that will accept
 * compressed data, and write your uncompressed source data to the
 * OCompressStream.
 *
 * Seeking is not supported.
 */
class EXPCL_PANDA_EXPRESS OCompressStream : public std::ostream {
PUBLISHED:
  INLINE OCompressStream();
  INLINE explicit OCompressStream(std::ostream *dest, bool owns_dest,
                                  int compression_level = 6);

#if _MSC_VER >= 1800
  INLINE OCompressStream(const OCompressStream &copy) = delete;
#endif

  INLINE OCompressStream &open(std::ostream *dest, bool owns_dest,
                               int compression_level = 6);
  INLINE OCompressStream &close();

private:
  ZStreamBuf _buf;
};

#include "zStream.I"

#endif  // HAVE_ZLIB


#endif
