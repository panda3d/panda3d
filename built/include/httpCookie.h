/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpCookie.h
 * @author drose
 * @date 2004-08-26
 */

#ifndef HTTPCOOKIE_H
#define HTTPCOOKIE_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even if you do not intend to use
// this to establish https connections; this is because it uses the OpenSSL
// library to portably handle all of the socket communications.

#if defined(HAVE_OPENSSL) || defined(__EMSCRIPTEN__)

#include "httpDate.h"
#include "urlSpec.h"

/**
 * A cookie sent from an HTTP server to be stored on the client and returned
 * when the path and/or domain matches.
 */
class EXPCL_PANDA_DOWNLOADER HTTPCookie {
PUBLISHED:
  INLINE HTTPCookie() = default;
  INLINE explicit HTTPCookie(const std::string &format, const URLSpec &url);
  INLINE explicit HTTPCookie(const std::string &name, const std::string &path,
                             const std::string &domain);
  INLINE ~HTTPCookie();

  INLINE void set_name(const std::string &name);
  INLINE const std::string &get_name() const;

  INLINE void set_value(const std::string &value);
  INLINE const std::string &get_value() const;

  INLINE void set_domain(const std::string &domain);
  INLINE const std::string &get_domain() const;

  INLINE void set_path(const std::string &path);
  INLINE const std::string &get_path() const;

  INLINE void set_expires(const HTTPDate &expires);
  INLINE void clear_expires();
  INLINE bool has_expires() const;
  INLINE HTTPDate get_expires() const;

  INLINE void set_secure(bool flag);
  INLINE bool get_secure() const;

  enum SameSite {
    SS_unspecified,
    SS_lax,
    SS_strict,
    SS_none,
  };

  INLINE void set_samesite(SameSite samesite);
  INLINE SameSite get_samesite() const;

  bool operator < (const HTTPCookie &other) const;
  void update_from(const HTTPCookie &other);

  bool parse_set_cookie(const std::string &format, const URLSpec &url);
  INLINE bool is_expired(const HTTPDate &now = HTTPDate::now()) const;
  bool matches_url(const URLSpec &url) const;

  void output(std::ostream &out) const;

PUBLISHED:
  MAKE_PROPERTY(name, get_name, set_name);
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(domain, get_domain, set_domain);
  MAKE_PROPERTY(path, get_path, set_path);
  MAKE_PROPERTY2(expires, has_expires, get_expires, set_expires, clear_expires);
  MAKE_PROPERTY(secure, get_secure, set_secure);

private:
  bool parse_cookie_param(const std::string &param, bool first_param);

  std::string _name;
  std::string _value;
  std::string _path;
  std::string _domain;
  HTTPDate _expires;
  bool _secure = false;
  SameSite _samesite = SS_unspecified;
};

std::ostream &operator << (std::ostream &out, HTTPCookie::SameSite samesite);
INLINE std::ostream &operator << (std::ostream &out, const HTTPCookie &cookie);

#include "httpCookie.I"

#endif  // HAVE_OPENSSL

#endif
