// Filename: p3dDownload.cxx
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

#include "p3dDownload.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DDownload::
P3DDownload() {
  _status = P3D_RC_in_progress;
  _http_status_code = 0;
  _total_data = 0;
  _total_expected_data = 0;
  _last_reported_time = 0;
  
  _canceled = false;
  _download_id = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DDownload::
~P3DDownload() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::set_url
//       Access: Public
//  Description: Supplies the source URL for the download.
////////////////////////////////////////////////////////////////////
void P3DDownload::
set_url(const string &url) {
  _url = url;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::cancel
//       Access: Public
//  Description: Cancels a running download.  download_finished() will
//               not be called, but the P3DDownload object itself will
//               eventually be deleted by its owning P3DInstance.
////////////////////////////////////////////////////////////////////
void P3DDownload::
cancel() {
  _canceled = true;
  if (_status == P3D_RC_in_progress) {
    _status = P3D_RC_generic_error;
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::feed_url_stream
//       Access: Public
//  Description: Called by P3DInstance as more data arrives from the
//               host.
////////////////////////////////////////////////////////////////////
bool P3DDownload::
feed_url_stream(P3D_result_code result_code,
                int http_status_code, 
                size_t total_expected_data,
                const unsigned char *this_data, 
                size_t this_data_size) {
  if (_canceled) {
    return false;
  }

  _status = result_code;
  _http_status_code = http_status_code;

  if (this_data_size != 0) {
    if (!receive_data(this_data, this_data_size)) {
      // Failure writing to disk or some such.
      _status = P3D_RC_generic_error;
      download_finished(false);
      return false;
    }

    _total_data += this_data_size;
  }

  _total_expected_data = max(total_expected_data, _total_data);

  download_progress();

  if (_status != P3D_RC_in_progress) {
    // We're done.
    download_finished(get_download_success());
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::receive_data
//       Access: Protected, Virtual
//  Description: Called as new data is downloaded.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DDownload::
receive_data(const unsigned char *this_data, size_t this_data_size) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::download_progress
//       Access: Protected, Virtual
//  Description: Intended to be overloaded to generate an occasional
//               callback as new data comes in.
////////////////////////////////////////////////////////////////////
void P3DDownload::
download_progress() {
  time_t now = time(NULL);
  if (now != _last_reported_time) {
    cerr << "Downloading " << get_url() << ": " 
         << int(get_download_progress() * 1000.0) / 10.0 << "\n";
    _last_reported_time = now;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DDownload::download_finished
//       Access: Protected, Virtual
//  Description: Intended to be overloaded to generate a callback
//               when the download finishes, either successfully or
//               otherwise.  The bool parameter is true if the
//               download was successful.
////////////////////////////////////////////////////////////////////
void P3DDownload::
download_finished(bool success) {
  cerr << "Downloading " << get_url() << ": " 
       << int(get_download_progress() * 1000.0) / 10.0 << "\n";
}
