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
//     Function: HTTPClient::set_try_all_direct
//       Access: Published
//  Description: If this is set true, then after a connection attempt
//               through a proxy fails, we always try a direct
//               connection, regardless of whether the host is listed
//               on the direct_host_spec list.  If this is false, a
//               direct attempt is not made when we have a proxy in
//               effect, even if the proxy fails.
////////////////////////////////////////////////////////////////////
INLINE void HTTPClient::
set_try_all_direct(bool try_all_direct) {
  _try_all_direct = try_all_direct;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_try_all_direct
//       Access: Published
//  Description: Returns whether a failed connection through a proxy
//               will be followed up by a direct connection attempt,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool HTTPClient::
get_try_all_direct() const {
  return _try_all_direct;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_http_version
//       Access: Published
//  Description: Specifies the version of HTTP that the client uses to
//               identify itself to the server.  The default is HV_11,
//               or HTTP 1.0; you can set this to HV_10 (HTTP 1.0) to
//               request the server use the older interface.
////////////////////////////////////////////////////////////////////
INLINE void HTTPClient::
set_http_version(HTTPEnum::HTTPVersion version) {
  _http_version = version;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_http_version
//       Access: Published
//  Description: Returns the client's current setting for HTTP
//               version.  See set_http_version().
////////////////////////////////////////////////////////////////////
INLINE HTTPEnum::HTTPVersion HTTPClient::
get_http_version() const {
  return _http_version;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_verify_ssl
//       Access: Published
//  Description: Specifies whether the client will insist on verifying
//               the identity of the servers it connects to via SSL
//               (that is, https).  
//
//               The parameter value is an enumerated type which
//               indicates the level of security to which the client
//               will insist upon.
////////////////////////////////////////////////////////////////////
INLINE void HTTPClient::
set_verify_ssl(HTTPClient::VerifySSL verify_ssl) {
  _verify_ssl = verify_ssl;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_verify_ssl
//       Access: Published
//  Description: Returns whether the client will insist on verifying
//               the identity of the servers it connects to via SSL
//               (that is, https).  See set_verify_ssl().
////////////////////////////////////////////////////////////////////
INLINE HTTPClient::VerifySSL HTTPClient::
get_verify_ssl() const {
  return _verify_ssl;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_cipher_list
//       Access: Published
//  Description: Specifies the set of ciphers that are to be made
//               available for SSL connections.  This is a string as
//               described in the ciphers(1) man page of the OpenSSL
//               documentation (or see
//               http://www.openssl.org/docs/apps/ciphers.html ).  If
//               this is not specified, the default is provided by the
//               Config file.  You may also specify "DEFAULT" to use
//               the built-in OpenSSL default value.
////////////////////////////////////////////////////////////////////
INLINE void HTTPClient::
set_cipher_list(const string &cipher_list) {
  _cipher_list = cipher_list;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_cipher_list
//       Access: Published
//  Description: Returns the set of ciphers as set by
//               set_cipher_list().  See set_cipher_list().
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPClient::
get_cipher_list() const {
  return _cipher_list;
}

