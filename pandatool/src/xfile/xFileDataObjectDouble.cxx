// Filename: xFileDataObjectDouble.cxx
// Created by:  drose (07Oct04)
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
  out << get_string_value();
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
    << get_string_value() << separator << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::set_int_value
//       Access: Protected, Virtual
//  Description: Sets the object's value as an integer, if this is
//               legal.
////////////////////////////////////////////////////////////////////
void XFileDataObjectDouble::
set_int_value(int int_value) {
  _value = (double)int_value;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::set_double_value
//       Access: Protected, Virtual
//  Description: Sets the object's value as a floating-point number,
//               if this is legal.
////////////////////////////////////////////////////////////////////
void XFileDataObjectDouble::
set_double_value(double double_value) {
  _value = double_value;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::get_int_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as an integer, if
//               it has one.
////////////////////////////////////////////////////////////////////
int XFileDataObjectDouble::
get_int_value() const {
  return (int)_value;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::get_double_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a double, if
//               it has one.
////////////////////////////////////////////////////////////////////
double XFileDataObjectDouble::
get_double_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectDouble::get_string_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a string, if
//               it has one.
////////////////////////////////////////////////////////////////////
string XFileDataObjectDouble::
get_string_value() const {
  // It's important to format with a decimal point, even if the value
  // is integral, since the DirectX .x reader differentiates betweens
  // doubles and integers on parsing.
  char buffer[128];
  sprintf(buffer, "%f", _value);
  
  return buffer;
}
