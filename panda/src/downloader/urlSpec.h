/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file urlSpec.h
 * @author drose
 * @date 2002-09-24
 */

#ifndef URLSPEC_H
#define URLSPEC_H

#include "pandabase.h"
#include "pnotify.h"

class Filename;

/**
 * A container for a URL, e.g.  "http://server:port/path".
 *
 * The URLSpec object is similar to a Filename in that it contains logic to
 * identify the various parts of a URL and return (or modify) them separately.
 */
class EXPCL_PANDA_DOWNLOADER URLSpec {
PUBLISHED:
  URLSpec();
  INLINE URLSpec(const std::string &url, bool server_name_expected = false);
  URLSpec(const URLSpec &url, const Filename &path);
  INLINE void operator = (const std::string &url);

  INLINE bool operator == (const URLSpec &other) const;
  INLINE bool operator != (const URLSpec &other) const;
  INLINE bool operator < (const URLSpec &other) const;
  int compare_to(const URLSpec &other) const;
  size_t get_hash() const;

  INLINE bool has_scheme() const;
  INLINE bool has_authority() const;
  INLINE bool has_username() const;
  INLINE bool has_server() const;
  INLINE bool has_port() const;
  INLINE bool has_path() const;
  INLINE bool has_query() const;

  std::string get_scheme() const;
  INLINE std::string get_authority() const;
  INLINE std::string get_username() const;
  INLINE std::string get_server() const;
  INLINE std::string get_port_str() const;
  uint16_t get_port() const;
  std::string get_server_and_port() const;
  bool is_default_port() const;
  static int get_default_port_for_scheme(const std::string &scheme);
  std::string get_path() const;
  INLINE std::string get_query() const;
  std::string get_path_and_query() const;
  INLINE bool is_ssl() const;

  INLINE const std::string &get_url() const;

  void set_scheme(const std::string &scheme);
  void set_authority(const std::string &authority);
  void set_username(const std::string &username);
  void set_server(const std::string &server);
  void set_port(const std::string &port);
  void set_port(uint16_t port);
  void set_server_and_port(const std::string &server_and_port);
  void set_path(const std::string &path);
  void set_query(const std::string &query);

  void set_url(const std::string &url, bool server_name_expected = false);

  INLINE operator const std::string & () const;
  INLINE const char *c_str() const;
  INLINE bool empty() const;
  INLINE operator bool() const;
  INLINE size_t length() const;
  INLINE size_t size() const;
  INLINE char operator [] (size_t n) const;

  bool input(std::istream &in);
  void output(std::ostream &out) const;

  static std::string quote(const std::string &source, const std::string &safe = "/");
  static std::string quote_plus(const std::string &source, const std::string &safe = "/");
  static std::string unquote(const std::string &source);
  static std::string unquote_plus(const std::string &source);

  MAKE_PROPERTY(scheme, get_scheme, set_scheme);
  MAKE_PROPERTY(authority, get_authority, set_authority);
  MAKE_PROPERTY(username, get_username, set_username);
  MAKE_PROPERTY(server, get_server, set_server);
  MAKE_PROPERTY(port, get_port, set_port);
  MAKE_PROPERTY(server_and_port, get_server_and_port, set_server_and_port);
  MAKE_PROPERTY(path, get_path, set_path);
  MAKE_PROPERTY(query, get_query, set_query);
  MAKE_PROPERTY(ssl, is_ssl);

private:
  void parse_authority();

  enum Flags {
    F_has_scheme     = 0x0001,
    F_has_authority  = 0x0002,
    F_has_username   = 0x0004,
    F_has_server     = 0x0008,
    F_has_port       = 0x0010,
    F_has_path       = 0x0020,
    F_has_query      = 0x0040,
  };

  std::string _url;
  uint16_t _port;
  int _flags;

  size_t _scheme_end;
  size_t _username_start;
  size_t _username_end;
  size_t _server_start;
  size_t _server_end;
  size_t _port_start;
  size_t _port_end;
  size_t _path_start;
  size_t _path_end;
  size_t _query_start;
};

INLINE std::istream &operator >> (std::istream &in, URLSpec &url);
INLINE std::ostream &operator << (std::ostream &out, const URLSpec &url);

#include "urlSpec.I"

#endif
