// Filename: p3dFileDownload.cxx
// Created by:  drose (11Jun09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "p3dFileDownload.h"
#include "p3dInstanceManager.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DFileDownload::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFileDownload::
P3DFileDownload() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileDownload::set_filename
//       Access: Public
//  Description: Supplies the target local filename for the download.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DFileDownload::
set_filename(const string &filename) {
  _filename = filename;

  return open_file();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileDownload::open_file
//       Access: Protected, Virtual
//  Description: Opens the local file for receiving the download.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DFileDownload::
open_file() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (!inst_mgr->mkfile_public(_filename)) {
    nout << "Unable to create " << _filename << "\n";
    return false;
  }
  
  _file.open(_filename.c_str(), ios::out | ios::ate | ios::binary);
  if (!_file) {
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileDownload::receive_data
//       Access: Protected, Virtual
//  Description: Called as new data is downloaded.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DFileDownload::
receive_data(const unsigned char *this_data, size_t this_data_size) {
  _file.write((const char *)this_data, this_data_size);

  if (!_file) {
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::download_finished
//       Access: Protected, Virtual
//  Description: Intended to be overloaded to generate a callback
//               when the download finishes, either successfully or
//               otherwise.  The bool parameter is true if the
//               download was successful.
////////////////////////////////////////////////////////////////////
void P3DFileDownload::
download_finished(bool success) {
  P3DDownload::download_finished(success);
  _file.close();
}
