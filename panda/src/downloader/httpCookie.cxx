/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpCookie.cxx
 * @author drose
 * @date 2004-08-26
 */

#include "httpCookie.h"

#ifdef HAVE_OPENSSL

#include "httpChannel.h"

#include <ctype.h>

using std::string;

/**
 * The sorting operator allows the cookies to be stored in a single
 * dictionary; it returns nonequal only if the cookies are different in name,
 * path, or domain.
 */
bool HTTPCookie::
operator < (const HTTPCookie &other) const {
  if (_domain != other._domain) {
    return _domain < other._domain;
  }

  if (_path != other._path) {
    // We use reverse sorting on the path, so that cookies with longer paths
    // will be sent to the server before cookies with shorter paths.
    return _path > other._path;
  }

  if (_name != other._name) {
    return _name < other._name;
  }

  return false;
}

/**
 * Assuming the operator < method, above, has already evaluated these two
 * cookies as equal, then assign the remaining values (value, expiration date,
 * secure flag) from the indicated cookie.  This is guaranteed not to change
 * the ordering of the cookie in a set, and so can be used to update an
 * existing cookie within a set with new values.
 */
void HTTPCookie::
update_from(const HTTPCookie &other) {
  nassertv(!(other < *this) && !(*this < other));

  _value = other._value;
  _expires = other._expires;
  _secure = other._secure;
}

/**
 * Separates out the parameter/value pairs of the Set-Cookie header and
 * assigns the values of the cookie appropriate.  Returns true if the header
 * is parsed correctly, false if something is not understood.
 */
bool HTTPCookie::
parse_set_cookie(const string &format, const URLSpec &url) {
  _name = string();
  _value = string();
  _domain = url.get_server();
  _path = url.get_path();
  _expires = HTTPDate();
  _secure = false;

  bool okflag = true;
  bool first_param = true;

  size_t start = 0;
  while (start < format.length() && isspace(format[start])) {
    start++;
  }
  size_t semicolon = format.find(';', start);

  while (semicolon != string::npos) {
    if (!parse_cookie_param(format.substr(start, semicolon - start),
                            first_param)) {
      okflag = false;
    }
    first_param = false;
    start = semicolon + 1;
    while (start < format.length() && isspace(format[start])) {
      start++;
    }
    semicolon = format.find(';', start);
  }

  if (!parse_cookie_param(format.substr(start), first_param)) {
    okflag = false;
  }

  return okflag;
}

/**
 * Returns true if the cookie is appropriate to send with the indicated URL
 * request, false otherwise.
 */
bool HTTPCookie::
matches_url(const URLSpec &url) const {
  if (_domain.empty()) {
    return false;
  }
  string server = url.get_server();
  if (server == _domain ||
      (string(".") + server) == _domain ||
      (server.length() > _domain.length() &&
       server.substr(server.length() - _domain.length()) == _domain &&
       (_domain[0] == '.' || server[server.length() - _domain.length() - 1] == '.'))) {
    // The domain matches.

    string path = url.get_path();
    if (path.length() >= _path.length() &&
        path.substr(0, _path.length()) == _path) {

      // The path matches too.
      if (_secure && !url.is_ssl()) {
        // Oops, can't send a secure cookie over a non-secure connection.
        return false;
      }

      return true;
    }
  }

  return false;
}

/**
 *
 */
void HTTPCookie::
output(std::ostream &out) const {
  out << _name << "=" << _value
      << "; path=" << _path << "; domain=" << _domain;

  if (has_expires()) {
    out << "; expires=" << _expires;
  }

  if (_secure) {
    out << "; secure";
  }
}

/**
 * Called internally by parse_set_cookie() with each parameter=value pair
 * split out from the header string.  first_param will be true for the first
 * parameter (which has special meaning).  This should return true on success,
 * false on failure.
 */
bool HTTPCookie::
parse_cookie_param(const string &param, bool first_param) {
  size_t equals = param.find('=');

  string key, value;
  if (equals == string::npos) {
    key = param;
  } else {
    key = param.substr(0, equals);
    value = param.substr(equals + 1);
  }

  if (first_param) {
    _name = key;
    _value = value;

  } else {
    key = HTTPChannel::downcase(key);
    if (key == "expires") {
      _expires = HTTPDate(value);
      if (!_expires.is_valid()) {
        return false;
      }

    } else if (key == "path") {
      _path = value;

    } else if (key == "domain") {
      _domain = HTTPChannel::downcase(value);

      // From RFC 2965: If an explicitly specified value does not start with a
      // dot, the user agent supplies a leading dot.
      if (!_domain.empty() && _domain[0] != '.') {
        _domain = string(".") + _domain;
      }

    } else if (key == "secure") {
      _secure = true;

    } else {
      return false;
    }
  }

  return true;
}

#endif  // HAVE_OPENSSL
