// Filename: httpClient.I
// Created by:  drose (24Sep02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE HTTPClient::
HTTPClient() {
  make_ctx();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE HTTPClient::
HTTPClient(const HTTPClient &copy) : _proxy(copy._proxy) {
  make_ctx();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void HTTPClient::
operator = (const HTTPClient &copy) {
  _proxy = copy._proxy;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE HTTPClient::
~HTTPClient() {
  SSL_CTX_free(_ssl_ctx);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_proxy
//       Access: Published
//  Description: Specifies the proxy URL to handle all http and
//               https requests.
////////////////////////////////////////////////////////////////////
INLINE void HTTPClient::
set_proxy(const URLSpec &proxy) {
  _proxy = proxy;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_proxy
//       Access: Published
//  Description: Returns the proxy URL to handle all http and
//               https requests.
////////////////////////////////////////////////////////////////////
INLINE const URLSpec &HTTPClient::
get_proxy() const {
  return _proxy;
}

