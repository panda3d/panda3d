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
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <filename.h>
#include <buffer.h>
#include <multifile.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : Extractor
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Extractor {
PUBLISHED:
  Extractor(void);
  Extractor(PT(Buffer) buffer);
  virtual ~Extractor(void);

  int initiate(Filename &source_file, const Filename &rel_path = "");
  int run(void);

  bool extract(Filename &source_file, const Filename &rel_path = "");

  INLINE float get_progress(void) const;

private:
  void init(PT(Buffer) buffer);
  void cleanup(void);

  bool _initiated;
  PT(Buffer) _buffer;

  ifstream _read_stream;
  int _source_file_length;
  Multifile *_mfile;
  int _total_bytes_read;
  bool _read_all_input;
  bool _handled_all_input;
  int _source_buffer_length;
  Filename _source_file;
  Filename _rel_path;
};

#include "extractor.I"

#endif
