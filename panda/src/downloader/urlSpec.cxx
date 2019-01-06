/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file urlSpec.cxx
 * @author drose
 * @date 2002-09-24
 */

#include "urlSpec.h"
#include "filename.h"
#include "string_utils.h"

#include <ctype.h>

using std::istream;
using std::ostream;
using std::ostringstream;
using std::setfill;
using std::setw;
using std::string;

/**
 *
 */
URLSpec::
URLSpec() {
  _port = 0;
  _flags = 0;
  _scheme_end = 0;
  _username_start = 0;
  _username_end = 0;
  _server_start = 0;
  _server_end = 0;
  _port_start = 0;
  _port_end = 0;
  _path_start = 0;
  _path_end = 0;
  _query_start = 0;
}

/**
 * Creates a URLSpec by appending a path to the end of the old URLSpec,
 * inserting an intervening forward slash if necessary.
 */
URLSpec::
URLSpec(const URLSpec &url, const Filename &path) {
  (*this) = url;
  if (!path.empty()) {
    string dirname = get_path();

    // Check if the path already ends in a slash.
    if (!dirname.empty() && dirname[dirname.size() - 1] == '/') {
      if (path[0] == '/') {
        // And the filename begins with one.  Remove the extra slash.
        dirname.resize(dirname.size() - 1);
      }
    } else {
      if (path[0] != '/') {
        // Neither has a slash, so insert one.
        dirname += '/';
      }
    }
    set_path(dirname + path.get_fullpath());
  }
}

/**
 * Returns a number less than zero if this URLSpec sorts before the other one,
 * greater than zero if it sorts after, or zero if they are equivalent.
 */
int URLSpec::
compare_to(const URLSpec &other) const {
  int cmp;
  if (has_scheme() != other.has_scheme()) {
    return (has_scheme() < other.has_scheme()) ? -1 : 1;
  }
  if (has_scheme()) {
    cmp = cmp_nocase(get_scheme(), other.get_scheme());
    if (cmp != 0) {
      return cmp;
    }
  }
  if (has_username() != other.has_username()) {
    return (has_username() < other.has_username()) ? -1 : 1;
  }
  if (has_username()) {
    cmp = get_username().compare(other.get_username());
    if (cmp != 0) {
      return cmp;
    }
  }
  if (has_server() != other.has_server()) {
    return (has_server() < other.has_server()) ? -1 : 1;
  }
  if (has_server()) {
    cmp = cmp_nocase(get_server(), other.get_server());
    if (cmp != 0) {
      return cmp;
    }
  }
  return get_path_and_query().compare(other.get_path_and_query());
}

/**
 *
 */
size_t URLSpec::
get_hash() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, _flags & (F_has_scheme | F_has_username | F_has_server));
  if (has_scheme()) {
    hash = string_hash::add_hash(hash, downcase(get_scheme()));
  }
  if (has_username()) {
    hash = string_hash::add_hash(hash, get_username());
  }
  if (has_server()) {
    hash = string_hash::add_hash(hash, downcase(get_server()));
  }
  hash = int_hash::add_hash(hash, get_port());
  hash = string_hash::add_hash(hash, get_path_and_query());
  return hash;
}

/**
 * Returns the scheme specified by the URL, or empty string if no scheme is
 * specified.
 */
string URLSpec::
get_scheme() const {
  if (has_scheme()) {
    return _url.substr(0, _scheme_end);
  }
  return string();
}

/**
 * Returns the port number specified by the URL, or the default port if not
 * specified.
 */
uint16_t URLSpec::
get_port() const {
  if (has_port()) {
    return _port;
  }
  return get_default_port_for_scheme(get_scheme());
}

/**
 * Returns true if the port number encoded in this URL is the default port
 * number for the scheme (or if there is no port number), or false if it is a
 * nonstandard port.
 */
bool URLSpec::
is_default_port() const {
  if (!has_port()) {
    return true;
  }
  return (_port == get_default_port_for_scheme(get_scheme()));
}

/**
 * Returns the default port number for the indicated scheme, or 0 if there is
 * no known default.
 */
int URLSpec::
get_default_port_for_scheme(const string &scheme) {
  if (scheme == "http" || scheme.empty()) {
    return 80;

  } else if (scheme == "https") {
    return 443;

  } else if (scheme == "socks") {
    return 1080;
  }

  return 0;
}

/**
 * Returns a string consisting of the server name, followed by a colon,
 * followed by the port number.  If the port number is not explicitly given in
 * the URL, this string will include the implicit port number.
 * If the server is an IPv6 address, it will be enclosed in square brackets.
 */
string URLSpec::
get_server_and_port() const {
  ostringstream strm;
  string server = get_server();
  if (server.find(':') != string::npos) {
    // Protect an IPv6 address by enclosing it in square brackets.
    strm << '[' << server << ']';

  } else {
    if (!has_port()) {
      return server;
    }
    strm << server;
  }
  if (has_port()) {
    strm << ":" << get_port();
  }
  return strm.str();
}

/**
 * Returns the path specified by the URL, or "/" if no path is specified.
 */
string URLSpec::
get_path() const {
  if (has_path()) {
    return _url.substr(_path_start, _path_end - _path_start);
  }
  return "/";
}

/**
 * Returns the path (or "/" if no path is specified), followed by the query if
 * it is specified.
 */
string URLSpec::
get_path_and_query() const {
  if (has_path()) {
    return _url.substr(_path_start);
  }
  if (has_query()) {
    return "/?" + get_query();
  }
  return "/";
}

/**
 * Replaces the scheme part of the URL specification.
 */
void URLSpec::
set_scheme(const string &scheme) {
  int length_adjust;

  // The scheme is always converted to lowercase.
  string lc_scheme;
  lc_scheme.reserve(scheme.length());
  for (string::const_iterator si = scheme.begin(); si != scheme.end(); ++si) {
    lc_scheme += tolower(*si);
  }

  if (lc_scheme.empty()) {
    // Remove the scheme specification.
    if (!has_scheme()) {
      return;
    }
    // Increment over the trailing colon so we can remove that too.
    _scheme_end++;
    length_adjust = -(int)_scheme_end;
    _url = _url.substr(_scheme_end);
    _flags &= ~F_has_scheme;

  } else if (!has_scheme()) {
    // Insert a new scheme specification.  The user may or may not have
    // specified a colon.
    if (lc_scheme[lc_scheme.length() - 1] == ':') {
      length_adjust = lc_scheme.length();
      _url = lc_scheme + _url;

    } else {
      length_adjust = lc_scheme.length() + 1;
      _url = lc_scheme + ":" + _url;
    }

    // Since the length_adjust flag, above, now accounts for the colon,
    // subtract one from _scheme_end (which should not include the colon).
    _scheme_end--;
    _flags |= F_has_scheme;

  } else {
    // Replace the existing scheme specification.  Since the existing scheme
    // will already be trailed by a colon, remove the colon from the string if
    // the user appended one.
    if (lc_scheme[lc_scheme.length() - 1] == ':') {
      lc_scheme = lc_scheme.substr(0, lc_scheme.length() - 1);
    }

    int old_length = (int)_scheme_end;
    length_adjust = scheme.length() - old_length;
    _url = lc_scheme + _url.substr(_scheme_end);
  }

  _scheme_end += length_adjust;
  _username_start += length_adjust;
  _username_end += length_adjust;
  _server_start += length_adjust;
  _server_end += length_adjust;
  _port_start += length_adjust;
  _port_end += length_adjust;
  _path_start += length_adjust;
  _path_end += length_adjust;
  _query_start += length_adjust;
}

/**
 * Replaces the authority part of the URL specification.  This includes the
 * username, server, and port.
 */
void URLSpec::
set_authority(const string &authority) {
  int length_adjust;
  int extra_slash_adjust = 0;

  if (authority.empty()) {
    // Remove the authority specification.
    if (!has_authority()) {
      return;
    }
    _username_start -= 2;
    length_adjust = -((int)_port_end - (int)_username_start);
    _url = _url.substr(0, _username_start) + _url.substr(_port_end);
    _flags &= ~(F_has_authority | F_has_username | F_has_server | F_has_port);

    _username_end = _username_start;
    _server_start = _username_start;
    _server_end = _username_start;
    _port_start = _username_start;

  } else if (!has_authority()) {
    // Insert a new authority specification.
    length_adjust = authority.length() + 2;

    string extra_slash;
    if (has_path() && _url[_path_start] != '/') {
      // If we have a path but it doesn't begin with a slash, it should.
      extra_slash = '/';
      extra_slash_adjust = 1;
    }
    _url = _url.substr(0, _username_start) + "//" + authority + extra_slash + _url.substr(_port_end);
    _flags |= F_has_authority;
    _username_start += 2;

  } else {
    // Replace an existing authority specification.
    int old_length = (int)_port_end - (int)_username_start;
    length_adjust = authority.length() - old_length;
    _url = _url.substr(0, _username_start) + authority + _url.substr(_port_end);
  }

  _port_end += length_adjust;
  _path_start += length_adjust;
  _path_end += length_adjust + extra_slash_adjust;
  _query_start += length_adjust + extra_slash_adjust;

  parse_authority();
}

/**
 * Replaces the username part of the URL specification.
 */
void URLSpec::
set_username(const string &username) {
  if (username.empty() && !has_authority()) {
    return;
  }
  string authority;

  if (!username.empty()) {
    authority = username + "@";
  }

  string server = get_server();
  if (server.find(':') != string::npos) {
    // Protect an IPv6 address by enclosing it in square brackets.
    authority += "[" + server + "]";
  } else {
    authority += server;
  }

  if (has_port()) {
    authority += ":";
    authority += get_port_str();
  }

  set_authority(authority);
}

/**
 * Replaces the server part of the URL specification.
 * Unlike set_server_and_port, this method does not require IPv6 addresses to
 * be enclosed in square brackets.
 */
void URLSpec::
set_server(const string &server) {
  if (server.empty() && !has_authority()) {
    return;
  }
  string authority;

  if (has_username()) {
    authority = get_username() + "@";
  }

  if (server.find(':') != string::npos) {
    // Protect an IPv6 address by enclosing it in square brackets.
    authority += "[" + server + "]";
  } else {
    authority += server;
  }

  if (has_port()) {
    authority += ":";
    authority += get_port_str();
  }

  set_authority(authority);
}

/**
 * Replaces the port part of the URL specification.
 */
void URLSpec::
set_port(const string &port) {
  if (port.empty() && !has_authority()) {
    return;
  }
  string authority;

  if (has_username()) {
    authority = get_username() + "@";
  }

  string server = get_server();
  if (server.find(':') != string::npos) {
    // Protect an IPv6 address by enclosing it in square brackets.
    authority += "[" + server + "]";
  } else {
    authority += server;
  }

  if (!port.empty()) {
    authority += ":";
    authority += port;
  }

  set_authority(authority);
}

/**
 * Replaces the port part of the URL specification, given a numeric port
 * number.
 */
void URLSpec::
set_port(uint16_t port) {
  set_port(format_string(port));
}

/**
 * Replaces the server and port parts of the URL specification simultaneously.
 * The input string should be of the form "server:port", or just "server" to
 * make the port number implicit.
 * Any IPv6 address must be enclosed in square brackets.
 */
void URLSpec::
set_server_and_port(const string &server_and_port) {
  if (server_and_port.empty() && !has_authority()) {
    return;
  }
  string authority;

  if (has_username()) {
    authority = get_username() + "@";
  }
  authority += server_and_port;
  set_authority(authority);
}

/**
 * Replaces the path part of the URL specification.
 */
void URLSpec::
set_path(const string &path) {
  int length_adjust;

  if (path.empty()) {
    // Remove the path specification.
    if (!has_path()) {
      return;
    }
    length_adjust = -((int)_path_end - (int)_path_start);
    _url = _url.substr(0, _path_start) + _url.substr(_path_end);
    _flags &= ~F_has_path;

  } else if (!has_path()) {
    // Insert a new path specification.
    string cpath = path;
    if (cpath[0] != '/') {
      // Paths must always begin with a slash.
      cpath = '/' + cpath;
    }
    length_adjust = cpath.length();

    _url = _url.substr(0, _path_start) + cpath + _url.substr(_path_end);
    _flags |= F_has_path;

  } else {
    // Replace an existing path specification.
    string cpath = path;
    if (cpath[0] != '/') {
      // Paths must always begin with a slash.
      cpath = '/' + cpath;
    }
    int old_length = (int)_path_end - (int)_path_start;
    length_adjust = cpath.length() - old_length;
    _url = _url.substr(0, _path_start) + cpath + _url.substr(_path_end);
  }

  _path_end += length_adjust;
  _query_start += length_adjust;
}

/**
 * Replaces the query part of the URL specification.
 */
void URLSpec::
set_query(const string &query) {
  if (query.empty()) {
    // Remove the query specification.
    if (!has_query()) {
      return;
    }
    _query_start--;
    _url = _url.substr(0, _query_start);
    _flags &= ~F_has_query;

  } else if (!has_query()) {
    // Insert a new query specification.
    _url = _url.substr(0, _query_start) + "?" + query;
    _flags |= F_has_query;
    _query_start++;

  } else {
    // Replace an existing query specification.
    _url = _url.substr(0, _query_start) + query;
  }
}

/**
 * Completely replaces the URL with the indicated string.  If
 * server_name_expected is true, it is a hint that an undecorated URL is
 * probably a server name, not a local filename.
 */
void URLSpec::
set_url(const string &url, bool server_name_expected) {
  size_t p, q;

  // Omit leading and trailing whitespace.
  p = 0;
  while (p < url.length() && isspace(url[p])) {
    p++;
  }
  q = url.length();
  while (q > p && isspace(url[q - 1])) {
    q--;
  }

  _url = url.substr(p, q - p);
  _flags = 0;

  if (url.empty()) {
    // No server name on an empty string.
    server_name_expected = false;
  }

  // First, replace backslashes with forward slashes, since this is a common
  // mistake among Windows users.  But don't do this after an embedded
  // question mark, which begins parameters sent directly to the host (and
  // maybe these parameters should include backslashes).
  for (p = 0; p < _url.length() && _url[p] != '?'; p++) {
    if (_url[p] == '\\') {
      _url[p] = '/';
    }
  }

  // Now normalize the percent-encoding sequences by making them uppercase.
  for (p = 0; p + 2 < _url.length();) {
    if (_url[p++] == '%') {
      if (_url[p] != '%') {
        _url[p] = toupper(_url[p]);
        ++p;
        _url[p] = toupper(_url[p]);
      }
      ++p;
    }
  }

  // What have we got?
  _flags = 0;
  _port = 0;

  // Look for the scheme specification.
  size_t start = 0;

  _scheme_end = start;
  size_t next = _url.find_first_of(":/", start);
  if (next < _url.length() - 1 && _url.substr(next, 2) == ":/") {
    // We have a scheme.
    _flags |= F_has_scheme;
    _scheme_end = next;

    // Ensure the scheme is lowercase.
    for (size_t p = 0; p < _scheme_end; ++p) {
      _url[p] = tolower(_url[p]);
    }

    start = next + 1;
  }

  // Look for the authority specification, which may include any of username,
  // server, andor port.
  _username_start = start;
  _username_end = start;
  _server_start = start;
  _server_end = start;
  _port_start = start;
  _port_end = start;

  // Try to determine if an authority is present.  It is will generally be
  // present if a scheme was present; also, we have a hint passed in from the
  // context as to whether we expect an authority (e.g.  a server name) to be
  // present.
  bool has_authority = (has_scheme() || server_name_expected);

  // We also know we have an authority if the url contains two slashes at this
  // point.
  bool leading_slashes =
    (start < _url.length() - 1 && _url.substr(start, 2) == "//");
  if (leading_slashes) {
    has_authority = true;
  }

  if (has_authority) {
    // Now that we know we have an authority, we should ensure there are two
    // slashes here, since there should be before the authority.
    if (!leading_slashes) {
      if (start < _url.length() && _url[start] == '/') {
        // Well, at least we had one slash.  Double it.
        _url = _url.substr(0, start + 1) + _url.substr(start);
      } else {
        // No slashes at all.  Insert them.
        _url = _url.substr(0, start) + "//" + _url.substr(start);
      }
    }

    // Begin the actual authority specification.
    start += 2;
    _flags |= F_has_authority;
    _username_start = start;
    _port_end = _url.find_first_of("/?", start);
    if (_port_end == string::npos) {
      _port_end = _url.length();
    }
    parse_authority();
    start = _port_end;
  }

  // Everything up to the ?, if any, is the path.
  _path_start = start;
  _path_end = start;
  if (start < _url.length() && url[start] != '?') {
    // We have a path.
    _flags |= F_has_path;
    _path_start = start;
    _path_end = _url.find("?", _path_start);
    if (_path_end == string::npos) {
      _path_end = _url.length();
    }
    start = _path_end;
  }

  // Everything after the ? is the query.
  _query_start = start;
  if (start < _url.length()) {
    nassertv(_url[start] == '?');
    _flags |= F_has_query;
    _query_start++;
  }
}

/**
 *
 */
bool URLSpec::
input(istream &in) {
  string url;
  in >> url;
  if (!in) {
    return false;
  }
  set_url(url);
  return true;
}

/**
 *
 */
void URLSpec::
output(ostream &out) const {
  out << get_url();
}

/**
 * Returns the source string with all "unsafe" characters quoted, making a
 * string suitable for placing in a URL.  Letters, digits, and the underscore,
 * comma, period, and hyphen characters, as well as any included in the safe
 * string, are left alone; all others are converted to hex representation.
 */
string URLSpec::
quote(const string &source, const string &safe) {
  ostringstream result;
  result << std::hex << setfill('0');

  for (string::const_iterator si = source.begin(); si != source.end(); ++si) {
    char ch = (*si);
    switch (ch) {
    case '_':
    case ',':
    case '.':
    case '-':
      // Safe character.
      result << ch;
      break;

    default:
      if (isalnum(ch)) {
        // Letters and digits are safe.
        result << ch;

      } else if (safe.find(ch) != string::npos) {
        // If it's listed in "safe", it's safe.
        result << ch;

      } else {
        // Otherwise, escape it.
        result << '%' << setw(2) << (int)ch;
      }
    }
  }

  return result.str();
}

/**
 * Behaves like quote() with the additional behavior of replacing spaces with
 * plus signs.
 */
string URLSpec::
quote_plus(const string &source, const string &safe) {
  ostringstream result;
  result << std::hex << setfill('0');

  for (string::const_iterator si = source.begin(); si != source.end(); ++si) {
    char ch = (*si);
    switch (ch) {
    case '_':
    case ',':
    case '.':
    case '-':
      // Safe character.
      result << ch;
      break;

    case ' ':
      result << '+';
      break;

    default:
      if (isalnum(ch)) {
        // Letters and digits are safe.
        result << ch;

      } else if (safe.find(ch) != string::npos) {
        // If it's listed in "safe", it's safe.
        result << ch;

      } else {
        // Otherwise, escape it.
        result << '%' << setw(2) << (int)ch;
      }
    }
  }

  return result.str();
}

/**
 * Reverses the operation of quote(): converts escaped characters of the form
 * "%xx" to their ascii equivalent.
 */
string URLSpec::
unquote(const string &source) {
  string result;

  size_t p = 0;
  while (p < source.length()) {
    if (source[p] == '%' && p + 2 < source.length()) {
      int hex = 0;
      p++;
      for (int i = 0; i < 2; i++) {
        int value;
        char ch = source[p + i];
        if (isdigit(ch)) {
          value = ch - '0';
        } else {
          value = tolower(ch) - 'a' + 10;
        }
        hex = (hex << 4) | value;
      }
      result += (char)hex;
      p += 2;

    } else {
      result += source[p];
      p++;
    }
  }

  return result;
}

/**
 * Reverses the operation of quote_plus(): converts escaped characters of the
 * form "%xx" to their ascii equivalent, and also converts plus signs to
 * spaces.
 */
string URLSpec::
unquote_plus(const string &source) {
  string result;

  size_t p = 0;
  while (p < source.length()) {
    if (source[p] == '%' && p + 2 < source.length()) {
      int hex = 0;
      p++;
      for (int i = 0; i < 2; i++) {
        int value;
        char ch = source[p + i];
        if (isdigit(ch)) {
          value = ch - '0';
        } else {
          value = tolower(ch) - 'a' + 10;
        }
        hex = (hex << 4) | value;
      }
      result += (char)hex;
      p += 2;

    } else if (source[p] == '+') {
      result += ' ';
      p++;

    } else {
      result += source[p];
      p++;
    }
  }

  return result;
}

/**
 * Assumes _url[_username_start .. _port_end - 1] is the authority component
 * if the URL, consisting of [username@]server[:port].  Parses out the three
 * pieces and updates the various _start and _end parameters accordingly.
 */
void URLSpec::
parse_authority() {
  _flags &= ~(F_has_username | F_has_server | F_has_port);

  if (!has_authority()) {
    return;
  }

  // Assume we don't have a username or port unless we find them.
  _username_end = _username_start;
  _port_start = _port_end;

  // We assume we have a server, even if it becomes the empty string.
  _flags |= F_has_server;
  _server_start = _username_start;
  _server_end = _port_end;

  // Is there a username?
  size_t at_sign = _url.find('@', _username_start);
  if (at_sign < _port_end) {
    // We have a username.
    _flags |= F_has_username;
    _username_end = at_sign;
    _server_start = at_sign + 1;
  }

  // Is this an IPv6 address in square brackets?
  size_t bracket = _url.find('[', _server_start);
  if (bracket < _port_end) {
    // We won't include the brackets in the server name.
    ++_server_start;

    bracket = _url.find(']');
    if (bracket < _server_end) {
      _server_end = bracket;

      // Is it followed directly by a port colon?
      size_t colon = _url.find(':', _server_end);
      if (colon < _port_end) {
        _port_start = colon + 1;
      }
    }
  } else {
    // Is there a port?
    size_t colon = _url.find(':', _server_start);
    if (colon < _port_end) {
      // Yep.
      _server_end = colon;
      _port_start = colon + 1;
    }
  }

  if (_port_start < _port_end) {
    _flags |= F_has_port;

    // Decode the port into an integer.  Don't bother to error check if it's
    // not really an integer.
    string port_str = _url.substr(_port_start, _port_end - _port_start);
    _port = (uint16_t)atoi(port_str.c_str());
  }

  // Make sure the server name is lowercase only.
  for (size_t si = _server_start; si != _server_end; ++si) {
    _url[si] = tolower(_url[si]);
  }

  // Also make sure the server name doesn't end with a dot.  It's happened!
  // Silly users.
  if (_server_end > _server_start && _url[_server_end - 1] == '.') {
    _url = _url.substr(0, _server_end - 1) + _url.substr(_server_end);
    _server_end--;
    _port_start--;
    _port_end--;
    _path_start--;
    _path_end--;
    _query_start--;
  }
}
