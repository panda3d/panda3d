/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpClient.I
 * @author drose
 * @date 2002-09-24
 */

/**
 * If this is set true, then after a connection attempt through a proxy fails,
 * we always try a direct connection, regardless of whether the host is listed
 * on the direct_host_spec list.  If this is false, a direct attempt is not
 * made when we have a proxy in effect, even if the proxy fails.
 */
INLINE void HTTPClient::
set_try_all_direct(bool try_all_direct) {
  _try_all_direct = try_all_direct;
}

/**
 * Returns whether a failed connection through a proxy will be followed up by
 * a direct connection attempt, false otherwise.
 */
INLINE bool HTTPClient::
get_try_all_direct() const {
  return _try_all_direct;
}

/**
 * Sets the filename of the pem-formatted file that will be read for the
 * client public and private keys if an SSL server requests a certificate.
 * Either this or set_client_certificate_pem() may be used to specify a client
 * certificate.
 */
INLINE void HTTPClient::
set_client_certificate_filename(const Filename &filename) {
  _client_certificate_filename = filename;
  _client_certificate_pem = std::string();
  unload_client_certificate();
}

/**
 * Sets the pem-formatted contents of the certificate that will be parsed for
 * the client public and private keys if an SSL server requests a certificate.
 * Either this or set_client_certificate_filename() may be used to specify a
 * client certificate.
 */
INLINE void HTTPClient::
set_client_certificate_pem(const std::string &pem) {
  _client_certificate_pem = pem;
  _client_certificate_filename = Filename();
  unload_client_certificate();
}

/**
 * Sets the passphrase used to decrypt the private key in the certificate
 * named by set_client_certificate_filename() or set_client_certificate_pem().
 */
INLINE void HTTPClient::
set_client_certificate_passphrase(const std::string &passphrase) {
  _client_certificate_passphrase = passphrase;
  unload_client_certificate();
}

/**
 * Specifies the version of HTTP that the client uses to identify itself to
 * the server.  The default is HV_11, or HTTP 1.0; you can set this to HV_10
 * (HTTP 1.0) to request the server use the older interface.
 */
INLINE void HTTPClient::
set_http_version(HTTPEnum::HTTPVersion version) {
  _http_version = version;
}

/**
 * Returns the client's current setting for HTTP version.  See
 * set_http_version().
 */
INLINE HTTPEnum::HTTPVersion HTTPClient::
get_http_version() const {
  return _http_version;
}

/**
 * Specifies whether the client will insist on verifying the identity of the
 * servers it connects to via SSL (that is, https).
 *
 * The parameter value is an enumerated type which indicates the level of
 * security to which the client will insist upon.
 */
INLINE void HTTPClient::
set_verify_ssl(HTTPClient::VerifySSL verify_ssl) {
  _verify_ssl = verify_ssl;
}

/**
 * Returns whether the client will insist on verifying the identity of the
 * servers it connects to via SSL (that is, https).  See set_verify_ssl().
 */
INLINE HTTPClient::VerifySSL HTTPClient::
get_verify_ssl() const {
  return _verify_ssl;
}

/**
 * Specifies the set of ciphers that are to be made available for SSL
 * connections.  This is a string as described in the ciphers(1) man page of
 * the OpenSSL documentation (or see
 * http://www.openssl.org/docs/apps/ciphers.html ).  If this is not specified,
 * the default is provided by the Config file.  You may also specify "DEFAULT"
 * to use the built-in OpenSSL default value.
 */
INLINE void HTTPClient::
set_cipher_list(const std::string &cipher_list) {
  _cipher_list = cipher_list;
}

/**
 * Returns the set of ciphers as set by set_cipher_list().  See
 * set_cipher_list().
 */
INLINE const std::string &HTTPClient::
get_cipher_list() const {
  return _cipher_list;
}

/**
 * Implements HTTPAuthorization::base64_encode().  This is provided here just
 * as a convenient place to publish it for access by the scripting language;
 * C++ code should probably use HTTPAuthorization directly.
 */
INLINE std::string HTTPClient::
base64_encode(const std::string &s) {
  return HTTPAuthorization::base64_encode(s);
}

/**
 * Implements HTTPAuthorization::base64_decode().  This is provided here just
 * as a convenient place to publish it for access by the scripting language;
 * C++ code should probably use HTTPAuthorization directly.
 */
INLINE std::string HTTPClient::
base64_decode(const std::string &s) {
  return HTTPAuthorization::base64_decode(s);
}
