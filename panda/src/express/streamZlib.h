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

#ifndef STREAM_ZLIB_H
#define STREAM_ZLIB_H

#include <memory>

#include "pandabase.h"

// This module is not compiled if zlib is not available.
#ifdef HAVE_ZLIB

#include "streamBufBase.h"

enum CompressionAlgorithm
{
  CA_zlib,
  CA_lz4,
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
class EXPCL_PANDA_EXPRESS IDecompressStreamZlib : public std::istream {
PUBLISHED:
  INLINE IDecompressStreamZlib(CompressionAlgorithm compression_algo);
  INLINE explicit IDecompressStreamZlib(std::istream *source, bool owns_source, CompressionAlgorithm compression_algo);

#if _MSC_VER >= 1800
  INLINE IDecompressStream(const IDecompressStream &copy) = delete;
#endif

<<<<<<< HEAD:panda/src/express/streamZlib.h
  INLINE IDecompressStreamZlib &open(std::istream *source, bool owns_source);
  INLINE IDecompressStreamZlib &close();

  static std::shared_ptr<StreamBufBase>& initialize_streambuf(std::shared_ptr<StreamBufBase> &buf_ptr, CompressionAlgorithm compression_algo);
=======
  INLINE IDecompressStream &open(std::istream *source, bool owns_source,
                                 std::streamsize source_length = -1,
                                 bool header=true);
  INLINE IDecompressStream &close();
>>>>>>> c9aedc2b4479ee517ca57e5e9451998160a6254f:panda/src/express/zStream.h

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
class EXPCL_PANDA_EXPRESS OCompressStreamZlib : public std::ostream {
PUBLISHED:
<<<<<<< HEAD:panda/src/express/streamZlib.h
  INLINE OCompressStreamZlib(CompressionAlgorithm compression_algo);
  INLINE explicit OCompressStreamZlib(std::ostream *dest, bool owns_dest,
                                      CompressionAlgorithm compression_algo, int compression_level = 6);
=======
  INLINE OCompressStream();
  INLINE explicit OCompressStream(std::ostream *dest, bool owns_dest,
                                  int compression_level = 6,
                                  bool header=true);
>>>>>>> c9aedc2b4479ee517ca57e5e9451998160a6254f:panda/src/express/zStream.h

#if _MSC_VER >= 1800
  INLINE OCompressStream(const OCompressStream &copy) = delete;
#endif

<<<<<<< HEAD:panda/src/express/streamZlib.h
  INLINE OCompressStreamZlib &open(std::ostream *dest, bool owns_dest,
                               int compression_level = 6);
  INLINE OCompressStreamZlib &close();

  static std::shared_ptr<StreamBufBase>& initialize_streambuf(std::shared_ptr<StreamBufBase> &buf_ptr, CompressionAlgorithm algorithm);
=======
  INLINE OCompressStream &open(std::ostream *dest, bool owns_dest,
                               int compression_level = 6,
                               bool header=true);
  INLINE OCompressStream &close();
>>>>>>> c9aedc2b4479ee517ca57e5e9451998160a6254f:panda/src/express/zStream.h

private:
  std::shared_ptr<StreamBufBase> _buf_ptr;
};

#include "streamZlib.I"

#endif  // HAVE_ZLIB


#endif
