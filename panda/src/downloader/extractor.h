// Filename: extractor.h
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
#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include "pandabase.h"
#include "filename.h"
#include "buffer.h"
#include "multifile.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : Extractor
// Description : This class automatically extracts the contents of a
//               Multifile to the current directory (or to a specified
//               directory) in the background.
//
//               It is designed to limit its use of system resources
//               and run unobtrusively in the background.  After
//               initiate(), each call to run() extracts another small
//               portion of the Multifile.  Call run() repeatedly
//               whenever you have spare cycles until run() returns
//               EU_success.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Extractor {
PUBLISHED:
  Extractor();
  ~Extractor();

  int initiate(const Filename &multifile_name, const Filename &extract_to = "");
  int run();

  bool extract(const Filename &multifile_name, const Filename &extract_to = "");

  INLINE float get_progress(void) const;

private:
  void cleanup();

  bool _initiated;
  Filename _multifile_name;
  Filename _extract_to;
  Multifile _multifile;

  int _subfile_index;
  size_t _subfile_pos;
  size_t _subfile_length;
  istream *_read;
  ofstream _write;
};

#include "extractor.I"

#endif
