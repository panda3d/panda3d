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

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_verify_ssl
//       Access: Published
//  Description: Specifies whether the client will insist on verifying
//               the identity of the servers it connects to via SSL
//               (that is, https).  If this is true (the default),
//               connections will only be allowed to trusted servers.
////////////////////////////////////////////////////////////////////
INLINE void HTTPClient::
set_verify_ssl(bool verify_ssl) {
  if (verify_ssl) {
    SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_PEER, NULL);
  } else {
    SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_NONE, NULL);
  }
  _verify_ssl = verify_ssl;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_verify_ssl
//       Access: Published
//  Description: Returns whether the client will insist on verifying
//               the identity of the servers it connects to via SSL
//               (that is, https).  See set_verify_ssl().
////////////////////////////////////////////////////////////////////
INLINE bool HTTPClient::
get_verify_ssl() const {
  return _verify_ssl;
}
