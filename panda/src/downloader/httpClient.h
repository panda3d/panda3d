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
#include "httpDocument.h"
#include "pointerTo.h"

#include <openssl/ssl.h>

class Filename;

////////////////////////////////////////////////////////////////////
//       Class : HTTPClient
// Description : Handles contacting an HTTP server and retrieving a
//               document.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HTTPClient {
PUBLISHED:
  INLINE HTTPClient();
  INLINE HTTPClient(const HTTPClient &copy);
  INLINE void operator = (const HTTPClient &copy);
  INLINE ~HTTPClient();

  INLINE void set_proxy(const URLSpec &proxy);
  INLINE const URLSpec &get_proxy() const;

  bool load_certificates(const Filename &filename);

  INLINE void set_verify_ssl(bool verify_ssl);
  INLINE bool get_verify_ssl() const;

  PT(HTTPDocument) get_document(const URLSpec &url, const string &body = string());

private:
  void make_ctx();
  static void initialize_ssl();
  static int load_verify_locations(SSL_CTX *ctx, const Filename &ca_file);

  BIO *get_http(const URLSpec &url, const string &body);
  BIO *get_https(const URLSpec &url, const string &body);
  BIO *get_http_proxy(const URLSpec &url, const string &body);
  BIO *get_https_proxy(const URLSpec &url, const string &body);

  BIO *make_https_connection(BIO *bio, const URLSpec &url) const;
  void send_get_request(BIO *bio, 
                        const string &path, const string &server, 
                        const string &body) const;

#ifndef NDEBUG
  static void show_send(const string &message);
#endif

#if defined(SSL_097) && !defined(NDEBUG)
  static void ssl_msg_callback(int write_p, int version, int content_type,
                               const void *buf, size_t len, SSL *ssl,
                               void *arg);
#endif

  URLSpec _proxy;
  SSL_CTX *_ssl_ctx;
  bool _verify_ssl;

  static bool _ssl_initialized;
};

#include "httpClient.I"

#endif  // HAVE_SSL

#endif
  
