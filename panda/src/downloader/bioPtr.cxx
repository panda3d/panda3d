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
  _server_name = url.get_server();
  _port = url.get_port();
  _bio = BIO_new_connect((char *)_server_name.c_str());
  BIO_set_conn_int_port(_bio, &_port);
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
