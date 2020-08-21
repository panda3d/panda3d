/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file extractor.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include "pandabase.h"
#include "filename.h"
#include "buffer.h"
#include "multifile.h"
#include "pointerTo.h"
#include "vector_int.h"

/**
 * This class automatically extracts the contents of a Multifile to the
 * current directory (or to a specified directory) in the background.
 *
 * It is designed to limit its use of system resources and run unobtrusively
 * in the background.  After specifying the files you wish to extract via
 * repeated calls to request_subfile(), begin the process by calling run()
 * repeatedly.  Each call to run() extracts another small portion of the
 * Multifile.  Call run() whenever you have spare cycles until run() returns
 * EU_success.
 */
class EXPCL_PANDA_DOWNLOADER Extractor {
PUBLISHED:
  Extractor();
  ~Extractor();

  bool set_multifile(const Filename &multifile_name);
  void set_extract_dir(const Filename &extract_dir);

  void reset();

  bool request_subfile(const Filename &subfile_name);
  int request_all_subfiles();

  int step();
  PN_stdfloat get_progress() const;

  bool run();

PUBLISHED:
  MAKE_PROPERTY(progress, get_progress);

private:
  Filename _multifile_name;
  PT(Multifile) _multifile;

  Filename _extract_dir;

  typedef vector_int Requests;
  Requests _requests;
  size_t _requests_total_length;

  bool _initiated;

  // These are used only while processing.
  int _request_index;
  int _subfile_index;
  size_t _subfile_pos;
  size_t _subfile_length;
  size_t _total_bytes_extracted;
  std::istream *_read;
  pofstream _write;
  Filename _subfile_filename;
};

#include "extractor.I"

#endif
