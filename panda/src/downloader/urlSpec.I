/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file urlSpec.I
 * @author drose
 * @date 2002-09-24
 */

/**

 */
INLINE URLSpec::
URLSpec(const string &url, bool server_name_expected) {
  set_url(url, server_name_expected);
}

/**

 */
INLINE URLSpec::
URLSpec(const URLSpec &copy) {
  (*this) = copy;
}

/**

 */
INLINE void URLSpec::
operator = (const string &url) {
  set_url(url);
}

/**

 */
INLINE bool URLSpec::
operator == (const URLSpec &other) const {
  return _url == other._url;
}

/**

 */
INLINE bool URLSpec::
operator != (const URLSpec &other) const {
  return !operator == (other);
}

/**

 */
INLINE bool URLSpec::
operator < (const URLSpec &other) const {
  return _url < other._url;
}

/**
 * Returns a number less than zero if this URLSpec sorts before the other one,
 * greater than zero if it sorts after, or zero if they are equivalent.
 */
INLINE int URLSpec::
compare_to(const URLSpec &other) const {
  return strcmp(_url.c_str(), other._url.c_str());
}

/**
 * Returns true if the URL specifies a scheme (e.g.  "http:"), false otherwise.
 */
INLINE bool URLSpec::
has_scheme() const {
  return (_flags & F_has_scheme) != 0;
}

/**
 * Returns true if the URL specifies an authority (this includes username,
 * server, and/or port), false otherwise.
 */
INLINE bool URLSpec::
has_authority() const {
  return (_flags & F_has_authority) != 0;
}

/**
 * Returns true if the URL specifies a username (and/or password), false
 * otherwise.
 */
INLINE bool URLSpec::
has_username() const {
  return (_flags & F_has_username) != 0;
}

/**
 * Returns true if the URL specifies a server name, false otherwise.
 */
INLINE bool URLSpec::
has_server() const {
  return (_flags & F_has_server) != 0;
}

/**
 * Returns true if the URL specifies a port number, false otherwise.
 */
INLINE bool URLSpec::
has_port() const {
  return (_flags & F_has_port) != 0;
}

/**
 * Returns true if the URL includes a path specification (that is, the
 * particular filename on the server to retrieve), false otherwise.
 */
INLINE bool URLSpec::
has_path() const {
  return (_flags & F_has_path) != 0;
}

/**
 * Returns true if the URL includes a query specification, false otherwise.
 */
INLINE bool URLSpec::
has_query() const {
  return (_flags & F_has_query) != 0;
}

/**
 * Returns the authority specified by the URL (this includes username, server,
 * and/or port), or empty string if no authority is specified.
 */
INLINE string URLSpec::
get_authority() const {
  return _url.substr(_username_start, _port_end - _username_start);
}

/**
 * Returns the username specified by the URL, if any.  This might also include a
 * password, e.g.  "username:password", although putting a password on the URL
 * is probably a bad idea.
 */
INLINE string URLSpec::
get_username() const {
  return _url.substr(_username_start, _username_end - _username_start);
}

/**
 * Returns the server name specified by the URL, if any.
 */
INLINE string URLSpec::
get_server() const {
  return _url.substr(_server_start, _server_end - _server_start);
}

/**
 * Returns the port specified by the URL as a string, or the empty string if no
 * port is specified.  Compare this with get_port(), which returns a default
 * port number if no port is specified.
 */
INLINE string URLSpec::
get_port_str() const {
  return _url.substr(_port_start, _port_end - _port_start);
}

/**
 * Returns the query specified by the URL, or empty string if no query is
 * specified.
 */
INLINE string URLSpec::
get_query() const {
  return _url.substr(_query_start);
}

/**
 * Returns true if the URL's scheme specifies an SSL-secured protocol such as
 * https, or false otherwise.
 */
INLINE bool URLSpec::
is_ssl() const {
  if (has_scheme() && _scheme_end > 0) {
    // If we have a scheme specification, assume it is SSL-secured if
    // it ends in "s", except for the special case of "socks".
    if (_url.substr(0, _scheme_end) == "socks") {
      return false;
    }
    return (_url[_scheme_end - 1] == 's');
  }

  // If we have no scheme specification, it's not SSL-secured.
  return false;
}

/**
 * Returns the complete URL specification.
 */
INLINE const string &URLSpec::
get_url() const {
  return _url;
}

/**

 */
INLINE URLSpec::
operator const string & () const {
  return _url;
}

/**

 */
INLINE const char *URLSpec::
c_str() const {
  return _url.c_str();
}

/**

 */
INLINE bool URLSpec::
empty() const {
  return _url.empty();
}

/**

 */
INLINE size_t URLSpec::
length() const {
  return _url.length();
}

/**

 */
INLINE char URLSpec::
operator [] (int n) const {
  nassertr(n >= 0 && n < (int)_url.length(), '\0');
  return _url[n];
}

INLINE istream &
operator >> (istream &in, URLSpec &url) {
  if (!url.input(in)) {
    in.clear(ios::failbit | in.rdstate());
  }
  return in;
}

INLINE ostream &
operator << (ostream &out, const URLSpec &url) {
  url.output(out);
  return out;
}
