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

#include "streamBufLz4.h"
#include "streamBufZlib.h"

/*
 *
 */
std::shared_ptr<StreamBufBase>& IDecompressStreamZlib::
initialize_streambuf(std::shared_ptr<StreamBufBase> &buf_ptr, CompressionAlgorithm compression_algo) {
#ifdef HAVE_ZLIB
  if (compression_algo == CompressionAlgorithm::CA_zlib) {
    buf_ptr = std::make_shared<StreamBufZlib>();
    return buf_ptr;
  }
#endif

#ifdef HAVE_LZ4
  if (algorithm == CompressionAlgorithm::CA_lz4) {
    buf_ptr = std::make_shared<StreamBufLz4>();
    return buf_ptr;
  }
#endif

  express_cat.error()
      << "Can't create streambuf object for IDecompressStream because the algorithm isn't available";
}


/*
 *
 */
std::shared_ptr<StreamBufBase>& OCompressStreamZlib::
initialize_streambuf(std::shared_ptr<StreamBufBase> &buf_ptr, CompressionAlgorithm algorithm) {
#ifdef HAVE_ZLIB
  if (algorithm == CompressionAlgorithm::CA_zlib) {
    buf_ptr = std::make_shared<StreamBufZlib>();
    return buf_ptr;
  }
#endif

#ifdef HAVE_LZ4
  if (algorithm == CompressionAlgorithm::CA_lz4) {
    buf_ptr = std::make_shared<StreamBufLz4>();
    return buf_ptr;
  }
#endif

  express_cat.error()
      << "Can't create streambuf object for OCompressStream because the algorithm isn't available";
}

/**
 *
 */
INLINE IDecompressStreamZlib::
IDecompressStreamZlib(CompressionAlgorithm compression_algo) :
  std::istream(IDecompressStreamZlib::initialize_streambuf(_buf_ptr, compression_algo).get()) {
}

/**
 *
 */
INLINE IDecompressStreamZlib::
IDecompressStreamZlib(std::istream *source, bool owns_source, CompressionAlgorithm compression_algo) :
  std::istream(IDecompressStreamZlib::initialize_streambuf(_buf_ptr, compression_algo).get()) {
  open(source, owns_source);
}

/**
 *
 */
INLINE IDecompressStreamZlib &IDecompressStreamZlib::
open(std::istream *source, bool owns_source) {
  clear((ios_iostate)0);
  _buf_ptr->open_read(source, owns_source);
  return *this;
}

/**
 * Resets the ZStream to empty, but does not actually close the source istream
 * unless owns_source was true.
 */
INLINE IDecompressStreamZlib &IDecompressStreamZlib::
close() {
  _buf_ptr->close_read();
  return *this;
}


/**
 *
 */
INLINE OCompressStreamZlib::
OCompressStreamZlib(CompressionAlgorithm compression_algo) :
  std::ostream(OCompressStreamZlib::initialize_streambuf(_buf_ptr, compression_algo).get()) {
}

/**
 *
 */
INLINE OCompressStreamZlib::
OCompressStreamZlib(std::ostream *dest, bool owns_dest, CompressionAlgorithm compression_algo, int compression_level) :
  std::ostream(OCompressStreamZlib::initialize_streambuf(_buf_ptr, compression_algo).get())
{
  open(dest, owns_dest, compression_level);
}

/**
 *
 */
INLINE OCompressStreamZlib &OCompressStreamZlib::
open(std::ostream *dest, bool owns_dest, int compression_level) {
  clear((ios_iostate)0);
  _buf_ptr->open_write(dest, owns_dest, compression_level);
  return *this;
}

/**
 * Resets the OCompressStream to empty, but does not actually close the dest ostream
 * unless owns_dest was true.
 */
INLINE OCompressStreamZlib &OCompressStreamZlib::
close() {
  _buf_ptr->close_write();
  return *this;
}
