/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dFileDownload.cxx
 * @author drose
 * @date 2009-06-11
 */

#include "p3dFileDownload.h"
#include "p3dInstanceManager.h"
#include "mkdir_complete.h"
#include "wstring_encode.h"

/**
 *
 */
P3DFileDownload::
P3DFileDownload() {
}

/**
 *
 */
P3DFileDownload::
P3DFileDownload(const P3DFileDownload &copy) :
  P3DDownload(copy)
{
  // We don't copy the filename.  You have to copy it yourself.
}

/**
 * Supplies the target local filename for the download.  Returns true on
 * success, false on failure.
 */
bool P3DFileDownload::
set_filename(const std::string &filename) {
  _filename = filename;

  return open_file();
}

/**
 * Opens the local file for receiving the download.  Returns true on success,
 * false on failure.
 */
bool P3DFileDownload::
open_file() {
  if (!mkfile_complete(_filename, nout)) {
    nout << "Failed to create " << _filename << "\n";
    return false;
  }

  _file.clear();
#ifdef _WIN32
  std::wstring filename_w;
  if (string_to_wstring(filename_w, _filename)) {
    _file.open(filename_w.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
  }
#else // _WIN32
  _file.open(_filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
#endif  // _WIN32
  if (!_file) {
    nout << "Failed to open " << _filename << " in write mode\n";
    return false;
  }

  return true;
}

/**
 * Closes the local file.
 */
void P3DFileDownload::
close_file() {
  _file.close();
}

/**
 * Called as new data is downloaded.  Returns true on success, false on
 * failure.
 */
bool P3DFileDownload::
receive_data(const unsigned char *this_data, size_t this_data_size) {
  _file.write((const char *)this_data, this_data_size);

  if (!_file) {
    return false;
  }

  return true;
}

/**
 * Intended to be overloaded to generate a callback when the download
 * finishes, either successfully or otherwise.  The bool parameter is true if
 * the download was successful.
 */
void P3DFileDownload::
download_finished(bool success) {
  P3DDownload::download_finished(success);
  _file.close();
}
