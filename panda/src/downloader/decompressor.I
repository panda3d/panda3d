// Filename: decompressor.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

#include "config_downloader.h"

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::get_progress
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE float Decompressor::
get_progress(void) const {
  if (_initiated == false) {
    downloader_cat.warning()
      << "Decompressor::get_progress() - Decompression has not been "
      << "initiated" << endl;
    return 0.0;
  }
  nassertr(_source_file_length > 0, 0.0);
  return ((float)_total_bytes_read / (float)_source_file_length);
}
