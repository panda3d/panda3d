// Filename: download.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

#include "config_downloader.h"

////////////////////////////////////////////////////////////////////
//     Function: Download::set_frequency
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void Download::
set_frequency(float frequency) {
  nassertv(frequency > 0.0);
  if (_frequency != frequency) {
    _frequency = frequency;
    _recompute_buffer = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Download::get_frequency
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE float Download::
get_frequency(void) const {
  return _frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: Download::set_byte_rate
//       Access: Public
//  Description: Note: modem speeds are reported in bits, so you 
//		 need to convert! 
////////////////////////////////////////////////////////////////////
INLINE void Download::
set_byte_rate(float bytes) {
  nassertv(bytes > 0.0);
  if (_byte_rate != bytes) {
    _byte_rate = bytes;
    _recompute_buffer = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Download::get_byte_rate
//       Access: Public
//  Description: Returns byte rate in bytes.
////////////////////////////////////////////////////////////////////
INLINE float Download::
get_byte_rate(void) const {
  return _byte_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: Download::set_disk_write_frequency
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Download::
set_disk_write_frequency(int frequency) {
  nassertv(frequency > 0);
  if (_disk_write_frequency != frequency) {
    _disk_write_frequency = frequency;
    _recompute_buffer = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Download::get_disk_write_frequency
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Download::
get_disk_write_frequency(void) const {
  return _disk_write_frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: Download::get_bytes_written
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Download::
get_bytes_written(void) const {
  nassertr(_current_status != NULL, 0);
  return _current_status->_total_bytes_written;
}

////////////////////////////////////////////////////////////////////
//     Function: Download::get_bytes_per_second
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float Download::
get_bytes_per_second(void) const {
  nassertr(_tlast - _tfirst > 0.0, 0.0);
  nassertr(_current_status != NULL, 0.0);
  return (float)((double)_current_status->_total_bytes / (_tlast - _tfirst));
}
