// Filename: patchfile.I
// Created by:  darren, mike (09Jan97)
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
    return 0.0;
  }
  nassertr(_result_file_length > 0, 0.0);
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
