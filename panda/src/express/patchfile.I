// Filename: patchfile.I
// Created by:  darren, mike (09Jan97)
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

//#include "config_downloader.h"

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::get_progress
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float Patchfile::
get_progress() const {
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
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Patchfile::
set_footprint_length(int length) {
  nassertv(_footprint_length > 0);
  _footprint_length = length;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::get_footprint_length
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Patchfile::
get_footprint_length() {
  return _footprint_length;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::reset_footprint_length
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Patchfile::
reset_footprint_length() {
  _footprint_length = _DEFAULT_FOOTPRINT_LENGTH;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::has_source_hash
//       Access: Published
//  Description: Returns true if the MD5 hash for the source file is
//               known.  (Some early versions of the patch file did
//               not store this information.)
////////////////////////////////////////////////////////////////////
INLINE bool Patchfile::
has_source_hash() const {
  return (_version_number >= 1);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::get_source_hash
//       Access: Published
//  Description: Returns the MD5 hash for the source file.
////////////////////////////////////////////////////////////////////
INLINE const HashVal &Patchfile::
get_source_hash() const {
  nassertr(has_source_hash(), _MD5_ofSource);
  return _MD5_ofSource;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::get_result_hash
//       Access: Published
//  Description: Returns the MD5 hash for the file after the patch has
//               been applied.
////////////////////////////////////////////////////////////////////
INLINE const HashVal &Patchfile::
get_result_hash() const {
  return _MD5_ofResult;
}
