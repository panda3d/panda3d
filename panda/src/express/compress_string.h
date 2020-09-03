/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compress_string.h
 * @author drose
 * @date 2009-08-09
 */

#ifndef COMPRESS_STRING_H
#define COMPRESS_STRING_H

#include "pandabase.h"
#include "filename.h"

#ifdef HAVE_ZLIB
#include "streamZlib.h"
#endif

#ifdef HAVE_LZ4
#include "streamLz4.h"
#endif

enum CompressionAlgorithm
{
  CA_zlib,
  CA_lz4,
};

#if defined (HAVE_ZLIB) or defined (HAVE_LZ4)

BEGIN_PUBLISH

EXPCL_PANDA_EXPRESS std::string
compress_string(const std::string &source, CompressionAlgorithm compression_algo, int compression_level);

EXPCL_PANDA_EXPRESS std::string
decompress_string(const std::string &source);

EXPCL_PANDA_EXPRESS bool
compress_file(const Filename &source, const Filename &dest, int compression_level);
EXPCL_PANDA_EXPRESS bool
decompress_file(const Filename &source, const Filename &dest);

EXPCL_PANDA_EXPRESS bool
compress_stream(std::istream &source, std::ostream &dest, CompressionAlgorithm compression_algo, int compression_level);
EXPCL_PANDA_EXPRESS bool
decompress_stream(std::istream &source, std::ostream &dest, CompressionAlgorithm compression_algo);

END_PUBLISH

std::shared_ptr<std::istream> create_Istream(CompressionAlgorithm compression_algo);
std::shared_ptr<std::ostream> create_Ostream(CompressionAlgorithm compression_algo);

#endif  // HAVE_ZLIB || HAVE_LZ4

#endif
