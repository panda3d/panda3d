// Filename: httpBasicAuthorization.cxx
// Created by:  drose (22Oct02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "httpBasicAuthorization.h"

#ifdef HAVE_OPENSSL

const string HTTPBasicAuthorization::_mechanism = "basic";

////////////////////////////////////////////////////////////////////
//     Function: HTTPBasicAuthorization::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
HTTPBasicAuthorization::
HTTPBasicAuthorization(const HTTPAuthorization::Tokens &tokens, 
                       const URLSpec &url, bool is_proxy) : 
  HTTPAuthorization(tokens, url, is_proxy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBasicAuthorization::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
HTTPBasicAuthorization::
~HTTPBasicAuthorization() {
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBasicAuthorization::get_mechanism
//       Access: Public, Virtual
//  Description: Returns the type of authorization mechanism,
//               represented as a string, e.g. "basic".
////////////////////////////////////////////////////////////////////
const string &HTTPBasicAuthorization::
get_mechanism() const {
  return _mechanism;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBasicAuthorization::generate
//       Access: Public, Virtual
//  Description: Generates a suitable authorization string to send
//               to the server, based on the data stored within this
//               object, for retrieving the indicated URL with the
//               given username:password.
////////////////////////////////////////////////////////////////////
string HTTPBasicAuthorization::
generate(HTTPEnum::Method, const string &,
         const string &username, const string &) {
  return "Basic " + base64_encode(username);
}

#endif  // HAVE_OPENSSL
