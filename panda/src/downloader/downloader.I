// Filename: downloader.I
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
//     Function: Downloader::get_file_size
//       Access: Published
//  Description: Returns the size of the file currently being
//               downloaded, in bytes, if it is known.  Returns 0 if
//               the file size is not known.
//
//               After a call to initiate(), this will return 0; at
//               some later point, when the first response from the
//               server is read, it may return the actual size of the
//               file if the server told us.  It is possible the
//               server will not tell us the size of the file, in
//               which case this will always return 0.
////////////////////////////////////////////////////////////////////
INLINE int Downloader::
get_file_size() const {
  return _file_size;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::set_frequency
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Downloader::
set_frequency(float frequency) {
  nassertv(frequency > 0.0);
  if (_frequency != frequency) {
    _frequency = frequency;
    _recompute_buffer = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::get_frequency
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float Downloader::
get_frequency(void) const {
  return _frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::set_byte_rate
//       Access: Public
//  Description: Note: modem speeds are reported in bits, so you
//               need to convert!
////////////////////////////////////////////////////////////////////
INLINE void Downloader::
set_byte_rate(float bytes) {
  nassertv(bytes > 0.0);
  if (_byte_rate != bytes) {
    _byte_rate = bytes;
    _recompute_buffer = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::get_byte_rate
//       Access: Public
//  Description: Returns byte rate in bytes.
////////////////////////////////////////////////////////////////////
INLINE float Downloader::
get_byte_rate(void) const {
  return _byte_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::set_disk_write_frequency
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Downloader::
set_disk_write_frequency(int frequency) {
  nassertv(frequency > 0);
  if (_disk_write_frequency != frequency) {
    _disk_write_frequency = frequency;
    _recompute_buffer = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::get_disk_write_frequency
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Downloader::
get_disk_write_frequency(void) const {
  return _disk_write_frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::get_bytes_written
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Downloader::
get_bytes_written(void) const {
  if (_ever_initiated == false) {
    downloader_cat.warning()
      << "Downloader::get_bytes_written() - Download has not been "
      << "initiated" << endl;
    return 0;
  }
  if (_current_status != NULL)
    return _current_status->_total_bytes_written;
  else
    return _total_bytes_written;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::get_bytes_requested
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Downloader::
get_bytes_requested(void) const {
  if (_ever_initiated == false) {
    downloader_cat.warning()
      << "Downloader::get_bytes_requested() - Download has not been "
      << "initiated" << endl;
    return 0;
  }
  if (_current_status != NULL)
    return _current_status->_total_bytes_requested;
  else
    return _total_bytes_requested;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::get_bytes_per_second
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float Downloader::
get_bytes_per_second(void) const {
  if (_ever_initiated == false) {
    downloader_cat.warning()
      << "Downloader::get_bytes_per_second() - Download has not been "
      << "initiated" << endl;
    return 0.0;
  }
  nassertr(_tlast - _tfirst > 0.0, 0.0);
  nassertr(_current_status != NULL, 0.0);
  return (float)((double)_current_status->_total_bytes / (_tlast - _tfirst));
}
