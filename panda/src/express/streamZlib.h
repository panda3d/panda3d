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

#ifndef STREAM_BASE_H
#define STREAM_BASE_H

#include <memory>

#include "pandabase.h"

// This module is not compiled if zlib is not available.
#if defined(HAVE_ZLIB) or defined(HAVE_LZ4)

#include "streamBufBase.h"
#include "streamBufZlib.h"
#include "streamBufLz4.h"

enum CompressionAlgorithm
{
    CA_zlib,
    CA_lz4
};

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
  INLINE IDecompressStream() = delete;
  INLINE IDecompressStream(CompressionAlgorithm compression_algo);
  INLINE explicit IDecompressStream(std::istream *source, bool owns_source, CompressionAlgorithm compression_algo);

#if _MSC_VER >= 1800
  INLINE IDecompressStream(const IDecompressStream &copy) = delete;
#endif

  INLINE IDecompressStream &open(std::istream *source, bool owns_source);
  INLINE IDecompressStream &close();

private:
  std::shared_ptr<StreamBufBase> _buf_ptr;
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
  INLINE OCompressStream() = delete;
  INLINE OCompressStream(CompressionAlgorithm compression_algo);
  INLINE explicit OCompressStream(std::ostream *dest, bool owns_dest,
                                  CompressionAlgorithm compression_algo,
                                  int compression_level = 6);

#if _MSC_VER >= 1800
  INLINE OCompressStream(const OCompressStream &copy) = delete;
#endif

  INLINE OCompressStream &open(std::ostream *dest, bool owns_dest,
                               int compression_level = 6);
  INLINE OCompressStream &close();

private:
  std::shared_ptr<StreamBufBase> _buf_ptr;
};

std::shared_ptr<StreamBufBase> create_buf_ptr(CompressionAlgorithm compression_algo);

#include "streamZlib.I"

#endif  // HAVE_ZLIB || HAVE_LZ4


#endif
