// Filename: xFileDataObjectDouble.cxx
// Created by:  drose (07Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "xFileDataObjectDouble.h"
#include "string_utils.h"
#include "indent.h"

TypeHandle XFileDataObjectDouble::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
XFileDataObjectDouble::
XFileDataObjectDouble(const XFileDataDef *data_def, double value) :
  XFileDataObject(data_def),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::output_data
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataObjectDouble::
output_data(ostream &out) const {
  out << as_string_value();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::write_data
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataObjectDouble::
write_data(ostream &out, int indent_level, const char *separator) const {
  indent(out, indent_level)
    << as_string_value() << separator << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::as_integer_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as an integer, if
//               it has one.
////////////////////////////////////////////////////////////////////
int XFileDataObjectDouble::
as_integer_value() const {
  return (int)_value;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::as_double_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a double, if
//               it has one.
////////////////////////////////////////////////////////////////////
double XFileDataObjectDouble::
as_double_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::as_string_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a string, if
//               it has one.
////////////////////////////////////////////////////////////////////
string XFileDataObjectDouble::
as_string_value() const {
  // It's important to format with a decimal point, even if the value
  // is integral, since the DirectX .x reader differentiates betweens
  // doubles and integers on parsing.
  char buffer[128];
  sprintf(buffer, "%f", _value);
  
  return buffer;
}
