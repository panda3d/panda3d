// Filename: httpClient.h
// Created by:  drose (24Sep02)
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

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even if you do not intend
// to use this to establish https connections; this is because it uses
// the OpenSSL library to portably handle all of the socket
// communications.

#ifdef HAVE_OPENSSL

#include "urlSpec.h"
#include "httpAuthorization.h"
#include "httpEnum.h"
#include "httpCookie.h"
#include "globPattern.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pmap.h"
#include "pset.h"
#include "referenceCount.h"
#include "openSSLWrapper.h"

class Filename;
class HTTPChannel;

////////////////////////////////////////////////////////////////////
//       Class : HTTPClient
// Description : Handles contacting an HTTP server and retrieving a
//               document.  Each HTTPClient object represents a
//               separate context, and stores its own list of cookies,
//               passwords, and certificates; however, a given
//               HTTPClient is capable of making multiple simultaneous
//               requests to the same or different servers.
//
//               It is up to the programmer whether one HTTPClient
//               should be used to retrieve all documents, or a
//               separate one should be created each time.  There is a
//               default, global HTTPClient available in
//               HTTPClient::get_global_ptr().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HTTPClient : public ReferenceCount {
PUBLISHED:
  HTTPClient();
  HTTPClient(const HTTPClient &copy);
  void operator = (const HTTPClient &copy);
  ~HTTPClient();

  static void init_random_seed();

  void set_proxy_spec(const string &proxy_spec);
  string get_proxy_spec() const;

  void set_direct_host_spec(const string &direct_host_spec);
  string get_direct_host_spec() const;

  INLINE void set_try_all_direct(bool try_all_direct);
  INLINE bool get_try_all_direct() const;

  void clear_proxy();
  void add_proxy(const string &scheme, const URLSpec &proxy);
  void clear_direct_host();
  void add_direct_host(const string &hostname);

  string get_proxies_for_url(const URLSpec &url) const;

  void set_username(const string &server, const string &realm, const string &username);
  string get_username(const string &server, const string &realm) const;

  void set_cookie(const HTTPCookie &cookie);
  bool clear_cookie(const HTTPCookie &cookie);
  void clear_all_cookies();
  bool has_cookie(const HTTPCookie &cookie) const;
  HTTPCookie get_cookie(const HTTPCookie &cookie) const;
  void copy_cookies_from(const HTTPClient &other);

  void write_cookies(ostream &out) const;
  void send_cookies(ostream &out, const URLSpec &url);
  
  INLINE void set_client_certificate_filename(const Filename &filename);
  INLINE void set_client_certificate_pem(const string &pem);
  INLINE void set_client_certificate_passphrase(const string &passphrase);
  bool load_client_certificate();

  bool add_preapproved_server_certificate_filename(const URLSpec &url, const Filename &filename);
  bool add_preapproved_server_certificate_pem(const URLSpec &url, const string &pem);
  bool add_preapproved_server_certificate_name(const URLSpec &url, const string &name);
  void clear_preapproved_server_certificates(const URLSpec &url);
  void clear_all_preapproved_server_certificates();

  INLINE void set_http_version(HTTPEnum::HTTPVersion version);
  INLINE HTTPEnum::HTTPVersion get_http_version() const;
  string get_http_version_string() const;
  static HTTPEnum::HTTPVersion parse_http_version_string(const string &version);

  bool load_certificates(const Filename &filename);

  enum VerifySSL {
    VS_no_verify,     // Don't care who we talk to
    VS_no_date_check, // Must identify certs, but old, expired certs are OK
    VS_normal         // Identify certs and also check expiration dates.
  };

  INLINE void set_verify_ssl(VerifySSL verify_ssl);
  INLINE VerifySSL get_verify_ssl() const;

  INLINE void set_cipher_list(const string &cipher_list);
  INLINE const string &get_cipher_list() const;

  PT(HTTPChannel) make_channel(bool persistent_connection);
  BLOCKING PT(HTTPChannel) post_form(const URLSpec &url, const string &body);
  BLOCKING PT(HTTPChannel) get_document(const URLSpec &url);
  BLOCKING PT(HTTPChannel) get_header(const URLSpec &url);

  INLINE static string base64_encode(const string &s);
  INLINE static string base64_decode(const string &s);

  static HTTPClient *get_global_ptr();

public:
  void get_proxies_for_url(const URLSpec &url, pvector<URLSpec> &proxies) const;

  SSL_CTX *get_ssl_ctx();

private:
  void check_preapproved_server_certificate(const URLSpec &url, X509 *cert,
                                            bool &cert_preapproved, bool &cert_name_preapproved) const;

  bool get_proxies_for_scheme(const string &scheme, 
                              pvector<URLSpec> &proxies) const;

  void add_http_username(const string &http_username);
  string select_username(const URLSpec &url, bool is_proxy, 
                         const string &realm) const;

  HTTPAuthorization *select_auth(const URLSpec &url, bool is_proxy,
                                 const string &last_realm);
  PT(HTTPAuthorization) generate_auth(const URLSpec &url, bool is_proxy,
                                      const string &challenge);

  void unload_client_certificate();

  static X509_NAME *parse_x509_name(const string &source);
  static bool x509_name_subset(X509_NAME *name_a, X509_NAME *name_b);

  static void split_whitespace(string &a, string &b, const string &c);

#if defined(SSL_097) && !defined(NDEBUG)
  static void ssl_msg_callback(int write_p, int version, int content_type,
                               const void *buf, size_t len, SSL *ssl,
                               void *arg);
#endif

  typedef pvector<URLSpec> Proxies;
  typedef pmap<string, Proxies> ProxiesByScheme;
  ProxiesByScheme _proxies_by_scheme;
  typedef pvector<GlobPattern> DirectHosts;
  DirectHosts _direct_hosts;
  bool _try_all_direct;

  HTTPEnum::HTTPVersion _http_version;
  VerifySSL _verify_ssl;
  string _cipher_list;

  typedef pmap<string, string> Usernames;
  Usernames _usernames;

  typedef pmap<string, PT(HTTPAuthorization)> Realms;
  class Domain {
  public:
    Realms _realms;
  };
  typedef pmap<string, Domain> Domains;
  Domains _proxy_domains, _www_domains;

  // Not a phash_set, since we want this to be maintained in order.
  typedef pset<HTTPCookie> Cookies;
  Cookies _cookies;

  Filename _client_certificate_filename;
  string _client_certificate_pem;
  string _client_certificate_passphrase;

  SSL_CTX *_ssl_ctx;
  bool _client_certificate_loaded;
  X509 *_client_certificate_pub;
  EVP_PKEY *_client_certificate_priv;

  typedef pvector<X509 *> ServerCerts;
  typedef pvector<X509_NAME *> ServerCertNames;
  class PreapprovedServerCert {
  public:
    ~PreapprovedServerCert();

    ServerCerts _certs;
    ServerCertNames _cert_names;
  };

  typedef pmap<string, PreapprovedServerCert> PreapprovedServerCerts;
  PreapprovedServerCerts _preapproved_server_certs;

  static PT(HTTPClient) _global_ptr;

  friend class HTTPChannel;
};

#include "httpClient.I"

#endif  // HAVE_OPENSSL

#endif
  
