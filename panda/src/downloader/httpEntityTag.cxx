/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpEntityTag.cxx
 * @author drose
 * @date 2003-01-28
 */

#include "httpEntityTag.h"

using std::string;


/**
 * This constructor accepts a string as formatted from an HTTP server (e.g.
 * the tag is quoted, with an optional W/ prefix.)
 */
HTTPEntityTag::
HTTPEntityTag(const string &text) {
  _weak = false;

  size_t p = 0;
  if (text.length() >= 2) {
    string sub = text.substr(0, 2);
    if (sub == "W/" || sub == "w/") {
      _weak = true;
      p = 2;
    }
  }

  // Unquote the string.
  bool quoted = false;
  if (p < text.length() && text[p] == '"') {
    quoted = true;
    p++;
  }
  while (p < text.length() && !(quoted && text[p] == '"')) {
    if (text[p] == '\\') {
      p++;
    }
    _tag += text[p];
    p++;
  }
}

/**
 * Returns the entity tag formatted for sending to an HTTP server (the tag is
 * quoted, with a conditional W prefix).
 */
string HTTPEntityTag::
get_string() const {
  std::ostringstream result;
  if (_weak) {
    result << "W/";
  }
  result << '"';

  for (string::const_iterator ti = _tag.begin(); ti != _tag.end(); ++ti) {
    switch (*ti) {
    case '"':
    case '\\':
      result << '\\';
      // fall through

    default:
      result << (*ti);
    }
  }

  result << '"';

  return result.str();
}
