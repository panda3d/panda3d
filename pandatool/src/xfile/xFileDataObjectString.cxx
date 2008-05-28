// Filename: xFileDataObjectString.cxx
// Created by:  drose (08Oct04)
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

#include "xFileDataObjectString.h"
#include "string_utils.h"
#include "indent.h"

TypeHandle XFileDataObjectString::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectString::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
XFileDataObjectString::
XFileDataObjectString(const XFileDataDef *data_def, const string &value) :
  XFileDataObject(data_def),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectString::output_data
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataObjectString::
output_data(ostream &out) const {
  enquote_string(out);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectString::write_data
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataObjectString::
write_data(ostream &out, int indent_level, const char *separator) const {
  indent(out, indent_level);
  enquote_string(out);
  out << separator << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectString::set_string_value
//       Access: Protected, Virtual
//  Description: Sets the object's value as a string, if this is
//               legal.
////////////////////////////////////////////////////////////////////
void XFileDataObjectString::
set_string_value(const string &string_value) {
  _value = string_value;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectString::get_string_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a string, if
//               it has one.
////////////////////////////////////////////////////////////////////
string XFileDataObjectString::
get_string_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectString::enquote_string
//       Access: Private
//  Description: Writes the string to the output stream without
//               quotation marks, quoting special characters as
//               needed.
////////////////////////////////////////////////////////////////////
void XFileDataObjectString::
enquote_string(ostream &out) const {
  // Actually, the XFile spec doesn't tell us how to escape special
  // characters within quotation marks.  We'll just take a stab in the
  // dark here.

  out << '"';
  string::const_iterator si;
  for (si = _value.begin(); si != _value.end(); ++si) {
    switch (*si) {
    case '\n':
      out << "\\n";
      break;

    case '\r':
      out << "\\r";
      break;

    case '"':
    case '\\':
      out << '\\' << (*si);
      break;

    default:
      out << (*si);
    }
  }
  out << '"';
}
