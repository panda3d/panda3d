// Filename: httpBasicAuthorization.cxx
// Created by:  drose (22Oct02)
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
