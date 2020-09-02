/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamBase.I
 * @author drose
 * @date 2002-08-05
 */

/**
 *
 */
INLINE IDecompressStream::
IDecompressStream(CompressionAlgorithm compression_algo) : std::istream(_buf_ptr) {
    _buf_ptr = create_buf_ptr(compression_algo);
}

/**
 *
 */
INLINE IDecompressStream::
IDecompressStream(std::istream *source, bool owns_source, CompressionAlgorithm compression_algo) : std::istream(_buf_ptr) {
  _buf_ptr = create_buf_ptr(compression_algo);
  open(source, owns_source);
}

/**
 *
 */
INLINE IDecompressStream &IDecompressStream::
open(std::istream *source, bool owns_source) {
  clear((ios_iostate)0);
  _buf_ptr->open_read(source, owns_source);
  return *this;
}

/**
 * Resets the ZStream to empty, but does not actually close the source istream
 * unless owns_source was true.
 */
INLINE IDecompressStream &IDecompressStream::
close() {
  _buf_ptr->close_read();
  return *this;
}


/**
 *
 */
INLINE OCompressStream::
OCompressStream(CompressionAlgorithm compression_algo) : std::ostream(_buf_ptr) {
}

/**
 *
 */
INLINE OCompressStream::
OCompressStream(std::ostream *dest, bool owns_dest, CompressionAlgorithm compression_algo, int compression_level) :
  std::ostream(_buf_ptr)
{
  open(dest, owns_dest, compression_level);
}

/**
 *
 */
INLINE OCompressStream &OCompressStream::
open(std::ostream *dest, bool owns_dest, int compression_level) {
  clear((ios_iostate)0);
  _buf_ptr->open_write(dest, owns_dest, compression_level);
  return *this;
}

/**
 * Resets the ZStream to empty, but does not actually close the dest ostream
 * unless owns_dest was true.
 */
INLINE OCompressStream &OCompressStream::
close() {
  _buf_ptr->close_write();
  return *this;
}

/**
 * Factory function for creating the shared_ptr of StreamBuf.
 */
std::shared_ptr<StreamBufBase> create_buf_ptr(CompressionAlgorithm compression_algo)
{
    switch (compression_algo) {
        case CA_zlib:
          return std::make_shared<StreamBufZlib>();
          break;
        case CA_lz4:
          return std::make_shared<StreamBufLZ4>();
    }
}
