/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpCookie.I
 * @author drose
 * @date 2004-08-26
 */

/**
 * Constructs an empty cookie.
 */
INLINE HTTPCookie::
HTTPCookie() :
  _secure(false)
{
}

/**
 * Constructs a cookie according to the indicated string, presumably the tag
 * of a Set-Cookie header.  There is no way to detect a formatting error in
 * the string with this constructor.
 */
INLINE HTTPCookie::
HTTPCookie(const std::string &format, const URLSpec &url) {
  parse_set_cookie(format, url);
}

/**
 * Constructs a cookie with the indicated name, path, and domain values, but
 * no other data.  This is most useful for looking up an existing cookie in
 * the HTTPClient.
 */
INLINE HTTPCookie::
HTTPCookie(const std::string &name, const std::string &path, const std::string &domain) :
  _name(name),
  _path(path),
  _domain(domain),
  _secure(false)
{
}

/**
 *
 */
INLINE HTTPCookie::
~HTTPCookie() {
}

/**
 *
 */
INLINE void HTTPCookie::
set_name(const std::string &name) {
  _name = name;
}

/**
 * Returns the name of the cookie.  This is the key value specified by the
 * server.
 */
INLINE const std::string &HTTPCookie::
get_name() const {
  return _name;
}

/**
 *
 */
INLINE void HTTPCookie::
set_value(const std::string &value) {
  _value = value;
}

/**
 * Returns the value of the cookie.  This is the arbitrary string associated
 * with the cookie's name, as specified by the server.
 */
INLINE const std::string &HTTPCookie::
get_value() const {
  return _value;
}

/**
 *
 */
INLINE void HTTPCookie::
set_domain(const std::string &domain) {
  _domain = domain;
}

/**
 *
 */
INLINE const std::string &HTTPCookie::
get_domain() const {
  return _domain;
}

/**
 *
 */
INLINE void HTTPCookie::
set_path(const std::string &path) {
  _path = path;
}

/**
 * Returns the prefix of the URL paths on the server for which this cookie
 * will be sent.
 */
INLINE const std::string &HTTPCookie::
get_path() const {
  return _path;
}

/**
 *
 */
INLINE void HTTPCookie::
set_expires(const HTTPDate &expires) {
  _expires = expires;
}

/**
 * Removes the expiration date on the cookie.
 */
INLINE void HTTPCookie::
clear_expires() {
  _expires = HTTPDate();
}

/**
 * Returns true if the cookie has an expiration date, false otherwise.
 */
INLINE bool HTTPCookie::
has_expires() const {
  return _expires.is_valid();
}

/**
 * Returns the expiration date of the cookie if it is set, or an invalid date
 * if it is not.
 */
INLINE HTTPDate HTTPCookie::
get_expires() const {
  return _expires;
}

/**
 *
 */
INLINE void HTTPCookie::
set_secure(bool secure) {
  _secure = secure;
}

/**
 * Returns true if the server has indicated this is a "secure" cookie which
 * should only be sent over an HTTPS channel.
 */
INLINE bool HTTPCookie::
get_secure() const {
  return _secure;
}

/**
 * Returns true if the cookie's expiration date is before the indicated date,
 * false otherwise.
 */
INLINE bool HTTPCookie::
is_expired(const HTTPDate &now) const {
  return _expires.is_valid() && _expires < now;
}

INLINE std::ostream &operator << (std::ostream &out, const HTTPCookie &cookie) {
  cookie.output(out);
  return out;
}
