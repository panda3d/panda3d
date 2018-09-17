/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpChannel.h
 * @author drose
 * @date 2002-09-24
 */

#ifndef HTTPCHANNEL_H
#define HTTPCHANNEL_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even if you do not intend to use
// this to establish https connections; this is because it uses the OpenSSL
// library to portably handle all of the socket communications.

#ifdef HAVE_OPENSSL

#include "httpClient.h"
#include "httpEnum.h"
#include "urlSpec.h"
#include "documentSpec.h"
#include "bioPtr.h"
#include "bioStreamPtr.h"
#include "pmap.h"
#include "pvector.h"
#include "pointerTo.h"
#include "config_downloader.h"
#include "filename.h"
#include "typedReferenceCount.h"

typedef struct bio_st BIO;

class Ramfile;
class HTTPClient;

/**
 * A single channel of communication from an HTTPClient.  This is similar to
 * the concept of a 'connection', except that HTTP is technically
 * connectionless; in fact, a channel may represent one unbroken connection or
 * it may transparently close and reopen a new connection with each request.
 *
 * A channel is conceptually a single thread of I/O. One document at a time
 * may be requested using a channel; a new document may (in general) not be
 * requested from the same HTTPChannel until the first document has been fully
 * retrieved.
 */
class EXPCL_PANDA_DOWNLOADER HTTPChannel : public TypedReferenceCount {
private:
  HTTPChannel(HTTPClient *client);

public:
  virtual ~HTTPChannel();

PUBLISHED:
  // get_status_code() will either return an HTTP-style status code >= 100
  // (e.g.  404), or one of the following values.  In general, these are
  // ordered from less-successful to more-successful.
  enum StatusCode {
    SC_incomplete = 0,
    SC_internal_error,
    SC_no_connection,
    SC_timeout,
    SC_lost_connection,
    SC_non_http_response,
    SC_invalid_http,
    SC_socks_invalid_version,
    SC_socks_no_acceptable_login_method,
    SC_socks_refused,
    SC_socks_no_connection,
    SC_ssl_internal_failure,
    SC_ssl_no_handshake,

    // No one returns this code, but StatusCode values higher than this are
    // deemed more successful than any generic HTTP response.
    SC_http_error_watermark,

    SC_ssl_invalid_server_certificate,
    SC_ssl_self_signed_server_certificate,
    SC_ssl_unexpected_server,

    // These errors are only generated after a download_to_*() call been
    // issued.
    SC_download_open_error,
    SC_download_write_error,
    SC_download_invalid_range,
  };

  INLINE HTTPClient *get_client() const;

  INLINE bool is_valid() const;
  INLINE bool is_connection_ready() const;

  INLINE const URLSpec &get_url() const;
  INLINE const DocumentSpec &get_document_spec() const;
  INLINE HTTPEnum::HTTPVersion get_http_version() const;
  INLINE const std::string &get_http_version_string() const;
  INLINE int get_status_code() const;
  std::string get_status_string() const;
  INLINE const std::string &get_www_realm() const;
  INLINE const std::string &get_proxy_realm() const;
  INLINE const URLSpec &get_redirect() const;
  std::string get_header_value(const std::string &key) const;

  INLINE int get_num_redirect_steps() const;
  INLINE const URLSpec &get_redirect_step(int n) const;
  MAKE_SEQ(get_redirect_steps, get_num_redirect_steps, get_redirect_step);

  INLINE void set_persistent_connection(bool persistent_connection);
  INLINE bool get_persistent_connection() const;
  bool will_close_connection() const;

  INLINE void set_allow_proxy(bool allow_proxy);
  INLINE bool get_allow_proxy() const;
  INLINE void set_proxy_tunnel(bool proxy_tunnel);
  INLINE bool get_proxy_tunnel() const;

  INLINE void set_connect_timeout(double timeout_seconds);
  INLINE double get_connect_timeout() const;
  INLINE void set_blocking_connect(bool blocking_connect);
  INLINE bool get_blocking_connect() const;

  INLINE void set_http_timeout(double timeout_seconds);
  INLINE double get_http_timeout() const;

  INLINE void set_skip_body_size(size_t skip_body_size);
  INLINE size_t get_skip_body_size() const;
  INLINE void set_idle_timeout(double idle_timeout);
  INLINE double get_idle_timeout() const;

  INLINE void set_download_throttle(bool download_throttle);
  INLINE bool get_download_throttle() const;

  INLINE void set_max_bytes_per_second(double max_bytes_per_second);
  INLINE double get_max_bytes_per_second() const;

  INLINE void set_max_updates_per_second(double max_updates_per_second);
  INLINE double get_max_updates_per_second() const;

  INLINE void set_content_type(std::string content_type);
  INLINE std::string get_content_type() const;

  INLINE void set_expected_file_size(size_t file_size);
  std::streamsize get_file_size() const;
  INLINE bool is_file_size_known() const;

  INLINE size_t get_first_byte_requested() const;
  INLINE size_t get_last_byte_requested() const;
  INLINE size_t get_first_byte_delivered() const;
  INLINE size_t get_last_byte_delivered() const;

  void write_headers(std::ostream &out) const;

  INLINE void reset();
  INLINE void preserve_status();

  INLINE void clear_extra_headers();
  INLINE void send_extra_header(const std::string &key, const std::string &value);

  BLOCKING INLINE bool get_document(const DocumentSpec &url);
  BLOCKING INLINE bool get_subdocument(const DocumentSpec &url,
                                       size_t first_byte, size_t last_byte);
  BLOCKING INLINE bool get_header(const DocumentSpec &url);
  BLOCKING INLINE bool post_form(const DocumentSpec &url, const std::string &body);
  BLOCKING INLINE bool put_document(const DocumentSpec &url, const std::string &body);
  BLOCKING INLINE bool delete_document(const DocumentSpec &url);
  BLOCKING INLINE bool get_trace(const DocumentSpec &url);
  BLOCKING INLINE bool connect_to(const DocumentSpec &url);
  BLOCKING INLINE bool get_options(const DocumentSpec &url);

  INLINE void begin_get_document(const DocumentSpec &url);
  INLINE void begin_get_subdocument(const DocumentSpec &url,
                                    size_t first_byte, size_t last_byte);
  INLINE void begin_get_header(const DocumentSpec &url);
  INLINE void begin_post_form(const DocumentSpec &url, const std::string &body);
  bool run();
  INLINE void begin_connect_to(const DocumentSpec &url);

  ISocketStream *open_read_body();
  void close_read_body(std::istream *stream) const;

  BLOCKING bool download_to_file(const Filename &filename, bool subdocument_resumes = true);
  BLOCKING bool download_to_ram(Ramfile *ramfile, bool subdocument_resumes = true);
  BLOCKING bool download_to_stream(std::ostream *strm, bool subdocument_resumes = true);
  SocketStream *get_connection();

  INLINE size_t get_bytes_downloaded() const;
  INLINE size_t get_bytes_requested() const;
  INLINE bool is_download_complete() const;

public:
  static std::string downcase(const std::string &s);
  void body_stream_destructs(ISocketStream *stream);

private:
  bool reached_done_state();
  bool run_try_next_proxy();
  bool run_connecting();
  bool run_connecting_wait();
  bool run_http_proxy_ready();
  bool run_http_proxy_request_sent();
  bool run_http_proxy_reading_header();
  bool run_socks_proxy_greet();
  bool run_socks_proxy_greet_reply();
  bool run_socks_proxy_connect();
  bool run_socks_proxy_connect_reply();
  bool run_setup_ssl();
  bool run_ssl_handshake();
  bool run_ready();
  bool run_request_sent();
  bool run_reading_header();
  bool run_start_direct_file_read();
  bool run_read_header();
  bool run_begin_body();
  bool run_reading_body();
  bool run_read_body();
  bool run_read_trailer();

  bool run_download_to_file();
  bool run_download_to_ram();
  bool run_download_to_stream();

  void begin_request(HTTPEnum::Method method, const DocumentSpec &url,
                     const std::string &body, bool nonblocking,
                     size_t first_byte, size_t last_byte);
  void reconsider_proxy();
  void reset_for_new_request();

  void finished_body(bool has_trailer);
  bool open_download_file();

  bool server_getline(std::string &str);
  bool server_getline_failsafe(std::string &str);
  bool server_get(std::string &str, size_t num_bytes);
  bool server_get_failsafe(std::string &str, size_t num_bytes);
  bool server_send(const std::string &str, bool secret);
  bool parse_http_response(const std::string &line);
  bool parse_http_header();
  bool parse_content_range(const std::string &content_range);

  void check_socket();

  void check_preapproved_server_certificate(X509 *cert, bool &cert_preapproved,
                                            bool &cert_name_preapproved) const;
  bool validate_server_name(X509 *cert);
  static bool match_cert_name(const std::string &cert_name, const std::string &hostname);
  static std::string get_x509_name_component(X509_NAME *name, int nid);

  void make_header();
  void make_proxy_request_text();
  void make_request_text();

  void reset_url(const URLSpec &old_url, const URLSpec &new_url);
  void store_header_field(const std::string &field_name, const std::string &field_value);

#ifndef NDEBUG
  static void show_send(const std::string &message);
#endif

  void reset_download_to();
  void close_download_stream();
  void reset_to_new();
  void reset_body_stream();
  void close_connection();

  static bool more_useful_status_code(int a, int b);

public:
  // This is declared public solely so we can make an ostream operator for it.
  enum State {
    S_new,
    S_try_next_proxy,
    S_connecting,
    S_connecting_wait,
    S_http_proxy_ready,
    S_http_proxy_request_sent,
    S_http_proxy_reading_header,
    S_socks_proxy_greet,
    S_socks_proxy_greet_reply,
    S_socks_proxy_connect,
    S_socks_proxy_connect_reply,
    S_setup_ssl,
    S_ssl_handshake,
    S_ready,
    S_request_sent,
    S_reading_header,
    S_start_direct_file_read,
    S_read_header,
    S_begin_body,
    S_reading_body,
    S_read_body,
    S_read_trailer,
    S_failure
  };

private:
  class StatusEntry {
  public:
    INLINE StatusEntry();
    int _status_code;
    std::string _status_string;
  };
  typedef pvector<URLSpec> Proxies;
  typedef pvector<StatusEntry> StatusList;

  HTTPClient *_client;
  Proxies _proxies;
  size_t _proxy_next_index;
  StatusList _status_list;
  URLSpec _proxy;
  PT(BioPtr) _bio;
  PT(BioStreamPtr) _source;
  bool _persistent_connection;
  bool _allow_proxy;
  bool _proxy_tunnel;
  double _connect_timeout;
  double _http_timeout;
  size_t _skip_body_size;
  double _idle_timeout;
  bool _blocking_connect;
  bool _download_throttle;
  double _max_bytes_per_second;
  double _max_updates_per_second;
  double _seconds_per_update;
  int _bytes_per_update;
  bool _nonblocking;
  bool _wanted_nonblocking;
  std::string _send_extra_headers;

  DocumentSpec _document_spec;
  DocumentSpec _request;
  HTTPEnum::Method _method;
  std::string request_path;
  std::string _header;
  std::string _body;
  std::string _content_type;
  bool _want_ssl;
  bool _proxy_serves_document;
  bool _proxy_tunnel_now;
  bool _server_response_has_no_body;
  size_t _first_byte_requested;
  size_t _last_byte_requested;
  size_t _first_byte_delivered;
  size_t _last_byte_delivered;
  int _connect_count;

  enum DownloadDest {
    DD_none,
    DD_file,
    DD_ram,
    DD_stream,
  };
  DownloadDest _download_dest;
  bool _subdocument_resumes;
  Filename _download_to_filename;
  Ramfile *_download_to_ramfile;
  std::ostream *_download_to_stream;

  int _read_index;

  HTTPEnum::HTTPVersion _http_version;
  std::string _http_version_string;
  StatusEntry _status_entry;
  URLSpec _redirect;

  std::string _proxy_realm;
  std::string _proxy_username;
  PT(HTTPAuthorization) _proxy_auth;

  std::string _www_realm;
  std::string _www_username;
  PT(HTTPAuthorization) _www_auth;

  // What type of response do we get to our HTTP request?
  enum ResponseType {
    RT_none,
    RT_hangup,       // immediately lost connection
    RT_non_http,     // something that wasn't an expected HTTP response
    RT_http_hangup,  // the start of an HTTP response, then a lost connection
    RT_http_complete // a valid HTTP response completed
  };
  ResponseType _response_type;

  // Not a phash_map, to maintain sorted order.
  typedef pmap<std::string, std::string> Headers;
  Headers _headers;

  size_t _expected_file_size;
  size_t _file_size;
  size_t _transfer_file_size;
  size_t _bytes_downloaded;
  size_t _bytes_requested;
  bool _got_expected_file_size;
  bool _got_file_size;
  bool _got_transfer_file_size;

  // These members are used to maintain the current state while communicating
  // with the server.  We need to store everything in the class object instead
  // of using local variables because in the case of nonblocking IO we have to
  // be able to return to the caller after any IO operation and resume later
  // where we left off.
  State _state;
  State _done_state;
  double _started_connecting_time;
  double _sent_request_time;
  bool _started_download;
  std::string _proxy_header;
  std::string _proxy_request_text;
  std::string _request_text;
  std::string _working_get;
  size_t _sent_so_far;
  std::string _current_field_name;
  std::string _current_field_value;
  ISocketStream *_body_stream;
  bool _owns_body_stream;
  BIO *_sbio;
  std::string _cipher_list;
  pvector<URLSpec> _redirect_trail;
  int _last_status_code;
  double _last_run_time;

  // RAU we find that we may need a little more time for the ssl handshake
  // when the phase files are downloading
  double _extra_ssl_handshake_time;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "HTTPChannel",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ChunkedStreamBuf;
  friend class IdentityStreamBuf;
  friend class HTTPClient;
};

std::ostream &operator << (std::ostream &out, HTTPChannel::State state);

#include "httpChannel.I"

#endif  // HAVE_OPENSSL

#endif
