// Filename: decompressor.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include "pandabase.h"

#ifdef HAVE_ZLIB

#include "filename.h"

class Ramfile;

////////////////////////////////////////////////////////////////////
//       Class : Decompressor
// Description : This manages run-time decompression of a
//               zlib-compressed stream, as a background or foreground
//               task.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Decompressor {
PUBLISHED:
  Decompressor();
  ~Decompressor();

  int initiate(const Filename &source_file);
  int initiate(const Filename &source_file, const Filename &dest_file);
  int run();

  bool decompress(const Filename &source_file);
  bool decompress(Ramfile &source_and_dest_file);

  float get_progress() const;

private:
  void cleanup(void);

  Filename _source_filename;
  
  istream *_source;
  istream *_decompress;
  ostream *_dest;

  size_t _source_length;
};

#include "decompressor.I"

#endif  // HAVE_ZLIB

#endif
