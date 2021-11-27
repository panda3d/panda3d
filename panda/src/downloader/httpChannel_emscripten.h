/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpChannel_emscripten.h
 * @author rdb
 * @date 2021-02-10
 */

#ifndef HTTPCHANNEL_EMSCRIPTEN_H
#define HTTPCHANNEL_EMSCRIPTEN_H

#include "pandabase.h"

#ifdef __EMSCRIPTEN__

#include "config_downloader.h"
#include "documentSpec.h"
#include "httpEnum.h"
#include "pmap.h"
#include "pointerTo.h"
#include "pvector.h"
#include "ramfile.h"
#include "typedReferenceCount.h"
#include "urlSpec.h"

class HTTPClient;

/**
 * This is a reduced implementation of HTTPChannel used on the web, which uses
 * XMLHttpRequest instead of OpenSSL.  It offers fewer features.
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

  INLINE const URLSpec &get_url() const;
  INLINE const DocumentSpec &get_document_spec() const;
  INLINE int get_status_code() const;
  INLINE std::string get_status_string() const;
  INLINE const std::string &get_www_realm() const;
  std::string get_header_value(const std::string &key) const;

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
  BLOCKING INLINE bool get_options(const DocumentSpec &url);

  INLINE void begin_get_document(const DocumentSpec &url);
  INLINE void begin_get_subdocument(const DocumentSpec &url,
                                    size_t first_byte, size_t last_byte);
  INLINE void begin_get_header(const DocumentSpec &url);
  INLINE void begin_post_form(const DocumentSpec &url, const std::string &body);
  bool run();

  BLOCKING bool download_to_ram(Ramfile *ramfile, bool subdocument_resumes = true);
  BLOCKING bool download_to_stream(std::ostream *strm, bool subdocument_resumes = true);

  INLINE size_t get_bytes_downloaded() const;
  INLINE size_t get_bytes_requested() const;
  INLINE bool is_download_complete() const;

private:
  enum State {
    S_unsent = 0,
    S_opened = 1,
    S_headers_received = 2,
    S_loading = 3,
    S_done = 4
  };
  State get_state() const;

  bool run_send();
  bool run_headers_received();

  bool begin_request(HTTPEnum::Method method, const DocumentSpec &url,
                     const std::string &body, bool nonblocking,
                     size_t first_byte, size_t last_byte);
  void reset_for_new_request();

  bool open_download_file();

  bool parse_content_range(const std::string &content_range);

  void reset_download_to();
  void close_download_stream();

private:
  class StatusEntry {
  public:
    INLINE StatusEntry();
    int _status_code;
    std::string _status_string;
  };
  typedef pvector<StatusEntry> StatusList;

  HTTPClient *_client;
  StatusList _status_list;
  URLSpec _proxy;

  typedef std::pair<std::string, std::string> ExtraHeader;
  pvector<ExtraHeader> _send_extra_headers;

  bool _nonblocking;
  bool _wanted_nonblocking;

  DocumentSpec _document_spec;
  DocumentSpec _request;
  HTTPEnum::Method _method;
  std::string request_path;
  std::string _header;
  std::string _body;
  std::string _content_type;
  size_t _first_byte_requested;
  size_t _last_byte_requested;
  size_t _first_byte_delivered;
  size_t _last_byte_delivered;

  enum DownloadDest {
    DD_none,
    DD_file,
    DD_ram,
    DD_stream,
  };
  DownloadDest _download_dest;
  bool _subdocument_resumes;
  //Filename _download_to_filename;
  Ramfile *_download_to_ramfile;
  std::ostream *_download_to_stream;

  StatusEntry _status_entry;

  std::string _www_realm;
  //std::string _www_username;
  //PT(HTTPAuthorization) _www_auth;

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
  friend class HTTPClient;
};

#include "httpChannel_emscripten.I"

#endif  // __EMSCRIPTEN__

#endif
