// Filename: patchfile.I
// Created by:  darren, mike (09Jan97)
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

//#include "config_downloader.h"

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::get_progress
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float Patchfile::
get_progress(void) const {
  if (false == _initiated) {
    express_cat.warning()
      << "Patchfile::get_progress() - Patch has not been initiated" << endl;
    return 0.0f;
  }
  nassertr(_result_file_length > 0, 0.0f);
  return ((float)_total_bytes_processed / (float)_result_file_length);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::set_footprint_length
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Patchfile::
set_footprint_length(int length) {
  nassertv(_footprint_length > 0);
  _footprint_length = length;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::get_footprint_length
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Patchfile::
get_footprint_length() {
  return _footprint_length;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::reset_footprint_length
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Patchfile::
reset_footprint_length() {
  _footprint_length = _DEFAULT_FOOTPRINT_LENGTH;
}
