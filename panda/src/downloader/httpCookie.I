// Filename: httpCookie.I
// Created by:  drose (26Aug04)
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


////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::Constructor
//       Access: Published
//  Description: Constructs an empty cookie.
////////////////////////////////////////////////////////////////////
INLINE HTTPCookie::
HTTPCookie() :
  _secure(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::Constructor
//       Access: Published
//  Description: Constructs a cookie according to the indicated
//               string, presumably the tag of a Set-Cookie header.
//               There is no way to detect a formatting error in the
//               string with this constructor.
////////////////////////////////////////////////////////////////////
INLINE HTTPCookie::
HTTPCookie(const string &format, const URLSpec &url) {
  parse_set_cookie(format, url);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::Constructor
//       Access: Published
//  Description: Constructs a cookie with the indicated name, path,
//               and domain values, but no other data.  This is most
//               useful for looking up an existing cookie in the
//               HTTPClient.
////////////////////////////////////////////////////////////////////
INLINE HTTPCookie::
HTTPCookie(const string &name, const string &path, const string &domain) :
  _name(name),
  _path(path),
  _domain(domain),
  _secure(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE HTTPCookie::
~HTTPCookie() {
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::set_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HTTPCookie::
set_name(const string &name) {
  _name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::get_name
//       Access: Published
//  Description: Returns the name of the cookie.  This is the key
//               value specified by the server.
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPCookie::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::set_value
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HTTPCookie::
set_value(const string &value) {
  _value = value;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::get_value
//       Access: Published
//  Description: Returns the value of the cookie.  This is the
//               arbitrary string associated with the cookie's name,
//               as specified by the server.
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPCookie::
get_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::set_domain
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HTTPCookie::
set_domain(const string &domain) {
  _domain = domain;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::get_domain
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPCookie::
get_domain() const {
  return _domain;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::set_path
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HTTPCookie::
set_path(const string &path) {
  _path = path;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::get_path
//       Access: Published
//  Description: Returns the prefix of the URL paths on the server for
//               which this cookie will be sent.
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPCookie::
get_path() const {
  return _path;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::set_expires
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HTTPCookie::
set_expires(const HTTPDate &expires) {
  _expires = expires;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::clear_expires
//       Access: Published
//  Description: Removes the expiration date on the cookie.
////////////////////////////////////////////////////////////////////
INLINE void HTTPCookie::
clear_expires() {
  _expires = HTTPDate();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::has_expires
//       Access: Published
//  Description: Returns true if the cookie has an expiration date,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool HTTPCookie::
has_expires() const {
  return _expires.is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::get_expires
//       Access: Published
//  Description: Returns the expiration date of the cookie if it is
//               set, or an invalid date if it is not.
////////////////////////////////////////////////////////////////////
INLINE HTTPDate HTTPCookie::
get_expires() const {
  return _expires;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::set_secure
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HTTPCookie::
set_secure(bool secure) {
  _secure = secure;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::get_secure
//       Access: Published
//  Description: Returns true if the server has indicated this is a
//               "secure" cookie which should only be sent over an
//               HTTPS channel.
////////////////////////////////////////////////////////////////////
INLINE bool HTTPCookie::
get_secure() const {
  return _secure;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPCookie::is_expired
//       Access: Published
//  Description: Returns true if the cookie's expiration date is
//               before the indicated date, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool HTTPCookie::
is_expired(const HTTPDate &now) const {
  return _expires.is_valid() && _expires < now;
}

INLINE ostream &operator << (ostream &out, const HTTPCookie &cookie) {
  cookie.output(out);
  return out;
}
