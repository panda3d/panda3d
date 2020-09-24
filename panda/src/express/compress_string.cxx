/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compress_string.cxx
 * @author drose
 * @date 2009-08-09
 */

#include <memory>

#include "compress_string.h"

#if defined(HAVE_ZLIB) or defined(HAVE_LZ4)
#include "virtualFileSystem.h"
#include "config_express.h"
#include "streamZlib.h"

using std::istream;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;

/**
 * Compress the indicated source string at the given compression level (1
 * through 9 for zlib and 1 through 12 for lz4).  Returns the compressed string.
 */
string
compress_string(const string &source, CompressionAlgorithm compression_algo, int compression_level) {
  ostringstream dest;
  {
    OCompressStreamZlib compress(compression_algo);
    compress.open(&dest, false, compression_level);
    compress.write(source.data(), source.length());

    if (compress.fail()) {
      return string();
    }
  }

  return dest.str();
}

/**
 * Decompresss the previously-compressed string()).  The return value is the
 * decompressed string.
 *
 * Note that a decompression error cannot easily be detected, and the return
 * value may simply be a garbage or truncated string.
 */
string
decompress_string(const string &source, CompressionAlgorithm compression_algo) {
  istringstream source_stream(source);
  ostringstream dest_stream;

  if (!decompress_stream(source_stream, dest_stream, compression_algo)) {
    return string();
  }

  return dest_stream.str();
}

/**
 * Compresss the data from the source file at the given compression level (1
 * through 9).  The source file is read in its entirety, and the compressed
 * results are written to the dest file, overwriting its contents.  The return
 * value is bool on success, or false on failure.
 */
EXPCL_PANDA_EXPRESS bool
compress_file(const Filename &source, const Filename &dest, CompressionAlgorithm compression_algo, int compression_level) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename source_filename = source;
  if (!source_filename.is_binary_or_text()) {
    // The default is binary, if not specified otherwise.
    source_filename.set_binary();
  }
  istream *source_stream = vfs->open_read_file(source_filename, false);
  if (source_stream == nullptr) {
    express_cat.info() << "Couldn't open file " << source_filename << "\n";
    return false;
  }

  Filename dest_filename = Filename::binary_filename(dest);
  ostream *dest_stream = vfs->open_write_file(dest_filename, false, true);
  if (dest_stream == nullptr) {
    express_cat.info() << "Couldn't open file " << dest_filename << "\n";
    vfs->close_read_file(source_stream);
    return false;
  }

  bool result = compress_stream(*source_stream, *dest_stream, compression_algo, compression_level);
  vfs->close_read_file(source_stream);
  vfs->close_write_file(dest_stream);
  return result;
}

/**
 * Decompresss the data from the source file.  The source file is read in its
 * entirety, and the decompressed results are written to the dest file,
 * overwriting its contents.  The return value is bool on success, or false on
 * failure.
 *
 * Note that a decompression error cannot easily be detected, and the output
 * may simply be a garbage or truncated string.
 */
EXPCL_PANDA_EXPRESS bool
decompress_file(const Filename &source, const Filename &dest, CompressionAlgorithm compression_algo) {
  Filename source_filename = Filename::binary_filename(source);
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *source_stream = vfs->open_read_file(source_filename, false);
  if (source_stream == nullptr) {
    express_cat.info() << "Couldn't open file " << source_filename << "\n";
    return false;
  }

  Filename dest_filename = dest;
  if (!dest_filename.is_binary_or_text()) {
    // The default is binary, if not specified otherwise.
    dest_filename.set_binary();
  }
  ostream *dest_stream = vfs->open_write_file(dest_filename, false, true);
  if (dest_stream == nullptr) {
    express_cat.info() << "Couldn't open file " << dest_filename << "\n";
    vfs->close_read_file(source_stream);
    return false;
  }

  bool result = decompress_stream(*source_stream, *dest_stream, compression_algo);
  vfs->close_read_file(source_stream);
  vfs->close_write_file(dest_stream);
  return result;
}

/**
 * Compresss the data from the source stream at the given compression level (1
 * through 9).  The source stream is read from its current position to the
 * end-of-file, and the compressed results are written to the dest stream.
 * The return value is bool on success, or false on failure.
 */
bool
compress_stream(istream &source, ostream &dest, CompressionAlgorithm compression_algo, int compression_level) {
  OCompressStreamZlib compress(compression_algo);
  compress.open(&dest, false, compression_level);

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  source.read(buffer, buffer_size);
  size_t count = source.gcount();
  while (count != 0) {
    compress.write(buffer, count);
    source.read(buffer, buffer_size);
    count = source.gcount();
  }
  compress.close();

  return (!source.fail() || source.eof()) && (!compress.fail());
}

/**
 * Decompresss the data from the previously-compressed source stream. The user
 * have to know which algorithm was used in this compressed source stream. The
 * source stream is read from its current position to the end-of-file, and the
 * decompressed results are written to the dest stream.  The return value is
 * bool on success, or false on failure.
 *
 * Note that a decompression error cannot easily be detected, and the output
 * may simply be a garbage or truncated string.
 */
bool
decompress_stream(istream &source, ostream &dest, CompressionAlgorithm compression_algo) {
  IDecompressStreamZlib decompress(compression_algo);

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  decompress.read(buffer, buffer_size);
  size_t count = decompress.gcount();
  while (count != 0) {
    dest.write(buffer, count);
    decompress.read(buffer, buffer_size);
    count = decompress.gcount();
  }

  return (!decompress.fail() || decompress.eof()) && (!dest.fail());
}
/*
std::shared_ptr<std::istream> create_Istream(CompressionAlgorithm compression_algo)
{
  if(compression_algo == CompressionAlgorithm::CA_zlib) {
#ifndef HAVE_ZLIB
    express_cat.error() << "zlib not present; could not decompress using zlib.\n"
#else
    return std::make_shared<IDecompressStreamZlib>();
#endif
  }


  if(compression_algo == CompressionAlgorithm::CA_zlib) {
#ifndef HAVE_LZ4
    express_cat.error() << "lz4 not present; could not decompress using zlib.\n"
#else
    return std::make_shared<IDecompressStreamLz4>();
#endif
  }
}

std::shared_ptr<std::ostream> create_Ostream(CompressionAlgorithm compression_algo)
{
  if(compression_algo == CompressionAlgorithm::CA_zlib) {
#ifndef HAVE_ZLIB
    express_cat.error() << "zlib not present; could not compress using zlib.\n"
#else
    return std::make_shared<OCompressStreamZlib>();
#endif
  }

  if(compression_algo == CompressionAlgorithm::CA_zlib) {
#ifndef HAVE_LZ4
    express_cat.error() << "lz4 not present; could not compress using zlib.\n";
#else
    return std::make_shared<OCompressStreamLz4>();
#endif
  }
}
*/

#endif // HAVE_ZLIB || HAVE_LZ4
