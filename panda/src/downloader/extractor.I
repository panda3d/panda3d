// Filename: extractor.I
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

#include "config_downloader.h"

////////////////////////////////////////////////////////////////////
//     Function: Extractor::get_progress
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float Extractor::
get_progress(void) const {
  if (_initiated == false) {
    downloader_cat.warning()
      << "Extractor::get_progress() - Extraction has not been initiated"
      << endl;
    return 0.0;
  }
  nassertr(_source_file_length > 0, 0.0);
  return ((float)_total_bytes_read / (float)_source_file_length);
}
