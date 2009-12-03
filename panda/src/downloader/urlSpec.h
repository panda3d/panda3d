// Filename: urlSpec.h
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

#ifndef URLSPEC_H
#define URLSPEC_H

#include "pandabase.h"
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//       Class : URLSpec
// Description : A container for a URL, e.g. "http://server:port/path".
//
//               The URLSpec object is similar to a Filename in that
//               it contains logic to identify the various parts of a
//               URL and return (or modify) them separately.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS URLSpec {
PUBLISHED:
  URLSpec();
  INLINE URLSpec(const string &url, bool server_name_expected = false);
  INLINE URLSpec(const URLSpec &copy);
  INLINE void operator = (const string &url);
  void operator = (const URLSpec &copy);

  INLINE bool operator == (const URLSpec &other) const;
  INLINE bool operator != (const URLSpec &other) const;
  INLINE bool operator < (const URLSpec &other) const;
  INLINE int compare_to(const URLSpec &other) const;

  INLINE bool has_scheme() const;
  INLINE bool has_authority() const;
  INLINE bool has_username() const;
  INLINE bool has_server() const;
  INLINE bool has_port() const;
  INLINE bool has_path() const;
  INLINE bool has_query() const;

  string get_scheme() const;
  INLINE string get_authority() const;
  INLINE string get_username() const;
  INLINE string get_server() const;
  INLINE string get_port_str() const;
  int get_port() const;
  string get_server_and_port() const;
  bool is_default_port() const;
  static int get_default_port_for_scheme(const string &scheme);
  string get_path() const;
  INLINE string get_query() const;
  string get_path_and_query() const;
  INLINE bool is_ssl() const;

  INLINE const string &get_url() const;

  void set_scheme(const string &scheme);
  void set_authority(const string &authority);
  void set_username(const string &username);
  void set_server(const string &server);
  void set_port(const string &port);
  void set_port(int port);
  void set_server_and_port(const string &server_and_port);
  void set_path(const string &path);
  void set_query(const string &query);

  void set_url(const string &url, bool server_name_expected = false);

  INLINE operator const string & () const;
  INLINE const char *c_str() const;
  INLINE bool empty() const;
  INLINE size_t length() const;
  INLINE char operator [] (int n) const;

  bool input(istream &in);
  void output(ostream &out) const;

  static string quote(const string &source, const string &safe = "/");
  static string quote_plus(const string &source, const string &safe = "/");
  static string unquote(const string &source);
  static string unquote_plus(const string &source);

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

  string _url;
  int _port;
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

INLINE istream &operator >> (istream &in, URLSpec &url);
INLINE ostream &operator << (ostream &out, const URLSpec &url);

#include "urlSpec.I"

#endif

