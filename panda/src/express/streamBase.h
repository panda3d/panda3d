/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamBase.h
 * @author drose
 * @date 2002-08-05
 */

#ifndef STREAM_BASE_H
#define STREAM_BASE_H

#include "pandabase.h"

class EXPCL_PANDA_EXPRESS IDecompressStreamBase : public std::istream {
PUBLISHED:
  INLINE IDecompressStreamBase();
  INLINE explicit IDecompressStreamBase(std::istream *source, bool owns_source);

#if _MSC_VER >= 1800
  INLINE IDecompressStreamLz4(const IDecompressStream &copy) = delete;
#endif

  INLINE virtual IDecompressStreamBase &open(std::istream *source, bool owns_source) = 0;
  INLINE virtual IDecompressStreamBase &close() = 0;
};

/**
 * An interface for  input stream object that uses zlib to compress (deflate) data to another
 * destination stream on-the-fly.
 *
 * Attach an OCompressStream to an existing ostream that will accept
 * compressed data, and write your uncompressed source data to the
 * OCompressStream.
 *
 * Seeking is not supported.
 */
class EXPCL_PANDA_EXPRESS OCompressStreamBase : public std::ostream {
PUBLISHED:
  INLINE OCompressStreamBase();
  INLINE explicit OCompressStreamBase(std::ostream *dest, bool owns_dest,
                                  int compression_level = 6);

#if _MSC_VER >= 1800
  INLINE OCompressStreamBase(const OCompressStream &copy) = delete;
#endif

  INLINE virtual OCompressStreamBase &open(std::ostream *dest, bool owns_dest,
                               int compression_level = 6) = 0;
  INLINE virtual OCompressStreamBase &close() = 0;
};
#endif // STREAM_BASE_H
