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
//     Function: P3DDownload::set_instance
//       Access: Public
//  Description: Specifies the particular P3DInstance that is
//               responsible for downloading this object.
////////////////////////////////////////////////////////////////////
inline void P3DDownload::
set_instance(P3DInstance *instance) {
  _instance = instance;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::get_instance
//       Access: Public
//  Description: Returns the particular P3DInstance that is
//               responsible for downloading this object.
////////////////////////////////////////////////////////////////////
inline P3DInstance *P3DDownload::
get_instance() const {
  return _instance;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::get_download_progress
//       Access: Public
//  Description: Returns an indication of the progress through the
//               download file, 0.0 to 1.0.  Returns 1.0 if the size
//               of the file is not known.
////////////////////////////////////////////////////////////////////
inline double P3DDownload::
get_download_progress() const {
  if (_total_expected_data == 0) {
    return 1.0;
  }

  return (double)_total_data / (double)_total_expected_data;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::is_download_progress_known
//       Access: Public
//  Description: Returns true if the download progress is known, or
//               false if it is unknown because the server hasn't told
//               us the total size it will be feeding us.  If this is
//               false, get_download_progress() will generally always
//               return 1.0; use get_total_bytes() to measure progress
//               in this case.
////////////////////////////////////////////////////////////////////
inline bool P3DDownload::
is_download_progress_known() const {
  return _progress_known;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::get_total_data
//       Access: Public
//  Description: Returns the total number of bytes downloaded so far.
////////////////////////////////////////////////////////////////////
inline size_t P3DDownload::
get_total_data() const {
  return _total_data;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::set_total_expected_data
//       Access: Public
//  Description: Sets the total number of bytes expected to be
//               downloaded.  This is used to compute the progress.
//               Normally, this can be set from the download server,
//               but there may be cases when the download server
//               doesn't accurately report it.
////////////////////////////////////////////////////////////////////
inline void P3DDownload::
set_total_expected_data(size_t expected_data) {
  _total_expected_data = expected_data;
  _progress_known = true;
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
//     Function: P3DDownload::get_download_terminated
//       Access: Public
//  Description: Returns true if the download has failed because the
//               instance is about to be shut down, or false if it
//               hasn't failed, or failed for some other reason.
////////////////////////////////////////////////////////////////////
inline bool P3DDownload::
get_download_terminated() const {
  return _status == P3D_RC_shutdown;
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
