// Filename: documentSpec.cxx
// Created by:  drose (28Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "documentSpec.h"


////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::compare_to
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DocumentSpec::
compare_to(const DocumentSpec &other) const {
  if (_flags != other._flags) {
    return (_flags - other._flags);
  }
  if (_request_mode != other._request_mode) {
    return (int)_request_mode - (int)other._request_mode;
  }
  int c = _url.compare_to(other._url);
  if (c != 0) {
    return c;
  }
  c = _tag.compare_to(other._tag);
  if (c != 0) {
    return c;
  }
  return _date.compare_to(other._date);
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void DocumentSpec::
output(ostream &out) const {
  out << get_url();
  if (has_tag()) {
    out << " (" << get_tag() << ")";
  }
  if (has_date()) {
    out << " " << get_date();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void DocumentSpec::
write(ostream &out) const {
  out << get_url() << "\n";
  if (has_tag()) {
    out << "  " << get_tag() << "\n";
  }
  if (has_date()) {
    out << "  " << get_date() << "\n";
  }
}
