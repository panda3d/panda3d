// Filename: httpClient.h
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

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even if you do not intend
// to use this to establish https connections; this is because it uses
// the OpenSSL library to portably handle all of the socket
// communications.

#ifdef HAVE_SSL

#include "urlSpec.h"
#include "pointerTo.h"

#include <openssl/ssl.h>

// Windows may define this macro inappropriately.
#ifdef X509_NAME
#undef X509_NAME
#endif

class Filename;
class HTTPChannel;

////////////////////////////////////////////////////////////////////
//       Class : HTTPClient
// Description : Handles contacting an HTTP server and retrieving a
//               document.  Each HTTPClient object represents a
//               separate context; it is up to the programmer whether
//               one HTTPClient should be used to retrieve all
//               documents, or a separate one should be created each
//               time.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HTTPClient {
PUBLISHED:
  HTTPClient();
  HTTPClient(const HTTPClient &copy);
  void operator = (const HTTPClient &copy);
  ~HTTPClient();

  INLINE void set_proxy(const URLSpec &proxy);
  INLINE const URLSpec &get_proxy() const;

  void set_username(const string &server, const string &realm, const string &username);
  string get_username(const string &server, const string &realm) const;

  enum HTTPVersion {
    HV_09,  // HTTP 0.9 or older
    HV_10,  // HTTP 1.0
    HV_11,  // HTTP 1.1
    HV_other,
  };

  INLINE void set_http_version(HTTPVersion version);
  INLINE HTTPVersion get_http_version() const;
  string get_http_version_string() const;
  static HTTPVersion parse_http_version_string(const string &version);

  bool load_certificates(const Filename &filename);

  enum VerifySSL {
    VS_no_verify,     // Don't care who we talk to
    VS_no_date_check, // Must identify certs, but old, expired certs are OK
    VS_normal         // Identify certs and also check expiration dates.
  };

  INLINE void set_verify_ssl(VerifySSL verify_ssl);
  INLINE VerifySSL get_verify_ssl() const;

  bool add_expected_server(const string &server_attributes);
  void clear_expected_servers();

  PT(HTTPChannel) make_channel(bool persistent_connection);
  PT(HTTPChannel) post_form(const URLSpec &url, const string &body);
  PT(HTTPChannel) get_document(const URLSpec &url);
  PT(HTTPChannel) get_header(const URLSpec &url);

public:
  SSL_CTX *get_ssl_ctx();

private:
  static void initialize_ssl();
  static int load_verify_locations(SSL_CTX *ctx, const Filename &ca_file);

  static X509_NAME *parse_x509_name(const string &source);

#if defined(SSL_097) && !defined(NDEBUG)
  static void ssl_msg_callback(int write_p, int version, int content_type,
                               const void *buf, size_t len, SSL *ssl,
                               void *arg);
#endif

  URLSpec _proxy;
  string _proxy_authorization;
  HTTPVersion _http_version;
  VerifySSL _verify_ssl;

  typedef pmap<string, string> Usernames;
  Usernames _usernames;

  // List of allowable SSL servers to connect to.  If the list is
  // empty, any server is acceptable.
  typedef pvector<X509_NAME *> ExpectedServers;
  ExpectedServers _expected_servers;

  SSL_CTX *_ssl_ctx;

  static bool _ssl_initialized;
  static X509_STORE *_x509_store;
  friend class HTTPChannel;
};

#include "httpClient.I"

#endif  // HAVE_SSL

#endif
  
