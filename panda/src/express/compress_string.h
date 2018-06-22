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

#ifdef HAVE_ZLIB

#include "filename.h"

BEGIN_PUBLISH

EXPCL_PANDA_EXPRESS std::string
compress_string(const std::string &source, int compression_level);

EXPCL_PANDA_EXPRESS std::string
decompress_string(const std::string &source);

EXPCL_PANDA_EXPRESS bool
compress_file(const Filename &source, const Filename &dest, int compression_level);
EXPCL_PANDA_EXPRESS bool
decompress_file(const Filename &source, const Filename &dest);

EXPCL_PANDA_EXPRESS bool
compress_stream(std::istream &source, std::ostream &dest, int compression_level);
EXPCL_PANDA_EXPRESS bool
decompress_stream(std::istream &source, std::ostream &dest);

END_PUBLISH

#endif  // HAVE_ZLIB

#endif
