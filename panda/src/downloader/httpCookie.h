// Filename: httpCookie.h
// Created by:  drose (26Aug04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef HTTPCOOKIE_H
#define HTTPCOOKIE_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even if you do not intend
// to use this to establish https connections; this is because it uses
// the OpenSSL library to portably handle all of the socket
// communications.

#ifdef HAVE_SSL

#include "httpDate.h"
#include "urlSpec.h"

////////////////////////////////////////////////////////////////////
//       Class : HTTPCookie
// Description : A cookie sent from an HTTP server to be stored on the
//               client and returned when the path and/or domain
//               matches.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HTTPCookie {
PUBLISHED:
  INLINE HTTPCookie();
  INLINE HTTPCookie(const string &format, const URLSpec &url);
  INLINE HTTPCookie(const string &name, const string &path, const string &domain);
  INLINE ~HTTPCookie();

  INLINE void set_name(const string &name);
  INLINE const string &get_name() const;

  INLINE void set_value(const string &value);
  INLINE const string &get_value() const;

  INLINE void set_domain(const string &domain);
  INLINE const string &get_domain() const;

  INLINE void set_path(const string &path);
  INLINE const string &get_path() const;

  INLINE void set_expires(const HTTPDate &expires);
  INLINE void clear_expires();
  INLINE bool has_expires() const;
  INLINE HTTPDate get_expires() const;

  INLINE void set_secure(bool flag);
  INLINE bool get_secure() const;

  bool operator < (const HTTPCookie &other) const;
  void update_from(const HTTPCookie &other);

  bool parse_set_cookie(const string &format, const URLSpec &url);
  INLINE bool is_expired(const HTTPDate &now = HTTPDate::now()) const;
  bool matches_url(const URLSpec &url) const;

  void output(ostream &out) const;

private:
  bool parse_cookie_param(const string &param, bool first_param);

  string _name;
  string _value;
  string _domain;
  string _path;
  HTTPDate _expires;
  bool _secure;
};

INLINE ostream &operator << (ostream &out, const HTTPCookie &cookie);

#include "httpCookie.I"

#endif  // HAVE_SSL

#endif
