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
//  Description: Returns the fraction of the Multifile extracted so
//               far.
////////////////////////////////////////////////////////////////////
INLINE float Extractor::
get_progress() const {
  nassertr(_initiated, 0.0f);
  return ((float)_subfile_index / (float)(_multifile.get_num_subfiles() + 1));
}
