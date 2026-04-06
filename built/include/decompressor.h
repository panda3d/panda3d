/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file decompressor.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include "pandabase.h"

#ifdef HAVE_ZLIB

#include "filename.h"

class Ramfile;

/**
 * This manages run-time decompression of a zlib-compressed stream, as a
 * background or foreground task.
 */
class EXPCL_PANDA_DOWNLOADER Decompressor {
PUBLISHED:
  Decompressor();
  ~Decompressor();

  int initiate(const Filename &source_file);
  int initiate(const Filename &source_file, const Filename &dest_file);
  int run();

  bool decompress(const Filename &source_file);
  bool decompress(Ramfile &source_and_dest_file);

  PN_stdfloat get_progress() const;

PUBLISHED:
  MAKE_PROPERTY(progress, get_progress);

private:
  void cleanup();

  Filename _source_filename;

  std::istream *_source;
  std::istream *_decompress;
  std::ostream *_dest;

  size_t _source_length;
};

#include "decompressor.I"

#endif  // HAVE_ZLIB

#endif
