// Filename: downloader.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

#include "config_downloader.h"

////////////////////////////////////////////////////////////////////
//     Function: Downloader::set_byte_rate
//       Access: Public
//  Description: Note: modem speeds are reported in bits, so you 
//               need to convert! 
////////////////////////////////////////////////////////////////////
INLINE void Downloader::
set_byte_rate(float bytes) {
  nassertv(bytes > 0.0);
  if (bytes == _byte_rate)
    return;
#ifdef HAVE_IPC
  _buffer_lock.lock();
#endif

  _new_byte_rate = bytes;

#ifdef HAVE_IPC
  _buffer_lock.unlock();
#endif
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
//     Function: Downloader::enable_download
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Downloader::
enable_download(bool val) {
  _download_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::is_download_enabled
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Downloader::
is_download_enabled(void) const {
  return _download_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::set_disk_write_frequency
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Downloader::
set_disk_write_frequency(int frequency) {
  nassertv(frequency > 0);
#ifdef HAVE_IPC
    _buffer_lock.lock();
#endif

  _new_disk_write_frequency = frequency;

#ifdef HAVE_IPC
    _buffer_lock.unlock();
#endif
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
//     Function: Downloader::get_last_attempt_stalled
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Downloader::
get_last_attempt_stalled(void) const {
  return _last_attempt_stalled;
}
