// Filename: p3dDownload.I
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


////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::get_url
//       Access: Public
//  Description: Returns the URL that we are querying.
////////////////////////////////////////////////////////////////////
const string &P3DDownload::
get_url() const {
  return _url;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::get_download_progress
//       Access: Public
//  Description: Returns an indication of the progress through the
//               download file, 0.0 to 1.0.  Returns -1 if the size of
//               the file is not known.
////////////////////////////////////////////////////////////////////
inline double P3DDownload::
get_download_progress() const {
  if (_total_expected_data == 0) {
    return -1.0;
  }

  return (double)_total_data / (double)_total_expected_data;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::get_download_finished
//       Access: Public
//  Description: Returns true if the download has finished, either
//               successfully or otherwise, or false if it is still in
//               progress.
////////////////////////////////////////////////////////////////////
inline bool P3DDownload::
get_download_finished() const {
  return _status != P3D_RC_in_progress;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::get_download_success
//       Access: Public
//  Description: Returns true if the download has finished
//               successfully, or false if it is still in progress or
//               if it has failed.
////////////////////////////////////////////////////////////////////
inline bool P3DDownload::
get_download_success() const {
  return _status == P3D_RC_done;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::set_download_id
//       Access: Public
//  Description: Called only by P3DInstance to set a unique ID for
//               this particular download object.
////////////////////////////////////////////////////////////////////
inline void P3DDownload::
set_download_id(int download_id) {
  _download_id = download_id;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::get_download_id
//       Access: Public
//  Description: Returns the unique ID set by the P3DInstance.
////////////////////////////////////////////////////////////////////
inline int P3DDownload::
get_download_id() const {
  return _download_id;
}
