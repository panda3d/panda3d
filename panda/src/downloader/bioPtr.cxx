// Filename: bioPtr.cxx
// Created by:  drose (15Oct02)
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

#include "bioPtr.h"

#ifdef HAVE_OPENSSL

#include "urlSpec.h"
#include "config_downloader.h"

////////////////////////////////////////////////////////////////////
//     Function: BioPtr::Constructor
//       Access: Public
//  Description: This flavor of the constructor automatically creates
//               a socket BIO and feeds it the server and port name
//               from the indicated URL.  It doesn't call
//               BIO_do_connect(), though.
////////////////////////////////////////////////////////////////////
BioPtr::
BioPtr(const URLSpec &url) {
  if (url.get_scheme() == "file") {
    // We're just reading a disk file.
    string filename = URLSpec::unquote(url.get_path());
#ifdef _WIN32 
    // On Windows, we have to munge the filename specially, because it's
    // been URL-munged.  It might begin with a leading slash as well as
    // a drive letter.  Clean up that nonsense.
    if (!filename.empty()) {
      if (filename[0] == '/' || filename[0] == '\\') {
        Filename fname = Filename::from_os_specific(filename.substr(1));
        if (fname.is_local()) {
          // Put the slash back on.
          fname = string("/") + fname.get_fullpath();
        }
        filename = fname.to_os_specific();
      }
    }
#endif  // _WIN32
    _server_name = "";
    _port = 0;
    _bio = BIO_new_file(filename.c_str(), "rb");

  } else {
    // A normal network-based URL.
    _server_name = url.get_server();
    _port = url.get_port();
    _bio = BIO_new_connect((char *)_server_name.c_str());
    BIO_set_conn_int_port(_bio, &_port);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BioPtr::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BioPtr::
~BioPtr() {
  if (_bio != (BIO *)NULL) {
    if (downloader_cat.is_debug() && !_server_name.empty()) {
      downloader_cat.debug()
        << "Dropping connection to " << _server_name << ":" << _port << "\n";
    }
      
    BIO_free_all(_bio);
    _bio = (BIO *)NULL;
  }
}

#endif  // HAVE_OPENSSL
