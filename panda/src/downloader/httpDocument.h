// Filename: httpDocument.h
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

#ifndef HTTPDOCUMENT_H
#define HTTPDOCUMENT_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even if you do not intend
// to use this to establish https connections; this is because it uses
// the OpenSSL library to portably handle all of the socket
// communications.

#ifdef HAVE_SSL

#include "httpClient.h"
#include "urlSpec.h"
#include "virtualFile.h"
#include "bioPtr.h"
#include "bioStreamPtr.h"
#include "pmap.h"
#include "pointerTo.h"
#include <openssl/ssl.h>

class HTTPClient;

////////////////////////////////////////////////////////////////////
//       Class : HTTPDocument
// Description : A single document retrieved from an HTTP server.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HTTPDocument : public VirtualFile {
private:
  HTTPDocument(HTTPClient *client);

  bool send_request(const string &method, const URLSpec &url, 
                    const string &body);
  bool send_request(const string &header, const string &body, 
                    bool allow_reconnect);

public:
  virtual ~HTTPDocument();

  virtual VirtualFileSystem *get_file_system() const;
  virtual Filename get_filename() const;

  virtual bool is_regular_file() const;
  virtual istream *open_read_file() const;

  bool will_close_connection() const;

PUBLISHED:
  INLINE bool is_valid() const;
  INLINE const URLSpec &get_url() const;
  INLINE HTTPClient::HTTPVersion get_http_version() const;
  INLINE const string &get_http_version_string() const;
  INLINE int get_status_code() const;
  INLINE const string &get_status_string() const;
  INLINE const string &get_realm() const;
  INLINE const URLSpec &get_redirect() const;
  string get_header_value(const string &key) const;

  INLINE void set_persistent_connection(bool persistent_connection);
  INLINE bool get_persistent_connection() const;

  INLINE size_t get_file_size() const;

  void write_headers(ostream &out) const;

  INLINE bool post_form(const URLSpec &url, const string &body);
  INLINE bool get_document(const URLSpec &url);
  INLINE bool get_header(const URLSpec &url);

  ISocketStream *read_body();

private:
  bool establish_connection();
  bool establish_http();
  bool establish_https();
  bool establish_http_proxy();
  bool establish_https_proxy();

  bool make_https_connection();
  bool verify_server(X509_NAME *subject) const;

  static string get_x509_name_component(X509_NAME *name, int nid);
  static bool x509_name_subset(X509_NAME *name_a, X509_NAME *name_b);

  void make_header(string &header, const string &method, 
                   const URLSpec &url, const string &body);
  void set_url(const URLSpec &url);
  void issue_request(const string &header, const string &body);
  void read_http_response();
  void store_header_field(const string &field_name, const string &field_value);
  bool get_authorization(string &authorization,
                         const string &authenticate_request, 
                         const URLSpec &url, bool is_proxy);

  static string downcase(const string &s);
  static string base64_encode(const string &s);
  static size_t scan_quoted_or_unquoted_string(string &result, const string &source, size_t start);

#ifndef NDEBUG
  static void show_send(const string &message);
#endif

  bool prepare_for_next(bool allow_reconnect);
  void free_bio();

  HTTPClient *_client;
  URLSpec _proxy;
  PT(BioPtr) _bio;
  PT(BioStreamPtr) _source;
  bool _persistent_connection;

  URLSpec _url;
  string _method;

  enum State {
    S_new,
    S_read_header,
    S_started_body,
    S_read_body,
    S_read_trailer
  };
  State _state;
  int _read_index;

  HTTPClient::HTTPVersion _http_version;
  string _http_version_string;
  int _status_code;
  string _status_string;
  string _realm;
  URLSpec _redirect;

  typedef pmap<string, string> Headers;
  Headers _headers;

  size_t _file_size;

  typedef pmap<string, string> Tokens;
  typedef pmap<string, Tokens> AuthenticationSchemes;
  static void parse_authentication_schemes(AuthenticationSchemes &schemes,
                                           const string &field_value);
  bool get_basic_authorization(string &authorization, const Tokens &tokens,
                               const URLSpec &url, bool is_proxy);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VirtualFile::init_type();
    register_type(_type_handle, "HTTPDocument",
                  VirtualFile::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ChunkedStreamBuf;
  friend class IdentityStreamBuf;
  friend class HTTPClient;
};

#include "httpDocument.I"

#endif  // HAVE_SSL

#endif


