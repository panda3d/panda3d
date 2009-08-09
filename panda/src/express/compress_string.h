// Filename: compress_string.h
// Created by:  drose (09Aug09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef COMPRESS_STRING_H
#define COMPRESS_STRING_H

#include "pandabase.h"

#ifdef HAVE_ZLIB

#include "filename.h"

BEGIN_PUBLISH

EXPCL_PANDAEXPRESS string
compress_string(const string &source, int compression_level);

EXPCL_PANDAEXPRESS string
decompress_string(const string &source);

EXPCL_PANDAEXPRESS bool
compress_file(const Filename &source, const Filename &dest, int compression_level);
EXPCL_PANDAEXPRESS bool
decompress_file(const Filename &source, const Filename &dest);

EXPCL_PANDAEXPRESS bool
compress_stream(istream &source, ostream &dest, int compression_level);
EXPCL_PANDAEXPRESS bool
decompress_stream(istream &source, ostream &dest);

END_PUBLISH

#endif  // HAVE_ZLIB

#endif
