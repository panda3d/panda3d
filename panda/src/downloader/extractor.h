// Filename: extractor.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
//               specifying the files you wish to extract via repeated
//               calls to request_subfile(), begin the process by
//               calling run() repeatedly.  Each call to run()
//               extracts another small portion of the Multifile.
//               Call run() whenever you have spare cycles until run()
//               returns EU_success.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Extractor {
PUBLISHED:
  Extractor();
  ~Extractor();

  bool set_multifile(const Filename &multifile_name);
  void set_extract_dir(const Filename &extract_dir);

  void reset();

  bool request_subfile(const Filename &subfile_name);
  int request_all_subfiles();

  int step();
  float get_progress(void) const;

  bool run();

private:
  Filename _multifile_name;
  PT(Multifile) _multifile;

  Filename _extract_dir;

  typedef pvector<int> Requests;
  Requests _requests;
  size_t _requests_total_length;
  
  bool _initiated;

  // These are used only while processing.
  int _request_index;
  int _subfile_index;
  size_t _subfile_pos;
  size_t _subfile_length;
  size_t _total_bytes_extracted;
  istream *_read;
  ofstream _write;
  Filename _subfile_filename;
};

#include "extractor.I"

#endif
