/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file documentSpec.cxx
 * @author drose
 * @date 2003-01-28
 */

#include "documentSpec.h"
#include "indent.h"


/**
 *
 */
int DocumentSpec::
compare_to(const DocumentSpec &other) const {
  if (_flags != other._flags) {
    return (_flags - other._flags);
  }
  int c = _url.compare_to(other._url);
  if (c != 0) {
    return c;
  }
  if (has_tag()) {
    c = _tag.compare_to(other._tag);
    if (c != 0) {
      return c;
    }
  }
  if (has_date()) {
    c = _date.compare_to(other._date);
    if (c != 0) {
      return c;
    }
  }

  // We don't consider _request_mode or _cache_control significant in the
  // comparison.

  return 0;
}

/**
 * Can be used to read in the DocumentSpec from a stream generated either by
 * output() or write().  Returns true on success, false on failure.
 */
bool DocumentSpec::
input(std::istream &in) {
  // First, clear the spec.
  (*this) = DocumentSpec();

  char ch;
  in >> ch;
  if (ch != '[') {
    return false;
  }
  in >> _url;
  in >> ch;
  if (ch == '(') {
    // Scan the tag, up to but not including the closing paren.
    std::string tag;
    in >> ch;
    while (!in.fail() && ch != ')') {
      tag += ch;
      // We want to include embedded whitespace, so we use get().
      ch = in.get();
    }
    set_tag(HTTPEntityTag(tag));

    // Now ch is the close paren following the tag; skip to the next
    // character.
    in >> ch;
  }

  // Scan the date, up to but not including the closing bracket.
  if (ch != ']') {
    std::string date;
    while (!in.fail() && ch != ']') {
      date += ch;
      ch = in.get();
    }

    set_date(HTTPDate(date));
    if (!get_date().is_valid()) {
      return false;
    }
  }

  return true;
}

/**
 *
 */
void DocumentSpec::
output(std::ostream &out) const {
  out << "[ " << get_url();
  if (has_tag()) {
    out << " (" << get_tag() << ")";
  }
  if (has_date()) {
    out << " " << get_date();
  }
  out << " ]";
}

/**
 *
 */
void DocumentSpec::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "[ " << get_url();
  if (has_tag()) {
    out << "\n";
    indent(out, indent_level + 2)
      << "(" << get_tag() << ")";
  }
  if (has_date()) {
    out << "\n";
    indent(out, indent_level + 2)
      << get_date();
  }
  out << " ]\n";
}
