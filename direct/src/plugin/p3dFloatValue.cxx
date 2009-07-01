// Filename: p3dFloatValue.cxx
// Created by:  drose (30Jun09)
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

#include "p3dFloatValue.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatValue::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFloatValue::
P3DFloatValue(double value) : 
  P3DValue(P3D_VT_float),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatValue::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFloatValue::
P3DFloatValue(const P3DFloatValue &copy) :
  P3DValue(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatValue::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DValue *P3DFloatValue::
make_copy() {
  return new P3DFloatValue(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatValue::get_bool
//       Access: Public, Virtual
//  Description: Returns the value value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DFloatValue::
get_bool() const {
  return (_value != 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatValue::get_int
//       Access: Public, Virtual
//  Description: Returns the value value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DFloatValue::
get_int() const {
  return (int)_value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatValue::get_float
//       Access: Public, Virtual
//  Description: Returns the value value coerced to a floating-point
//               value, if possible.
////////////////////////////////////////////////////////////////////
double P3DFloatValue::
get_float() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatValue::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DFloatValue::
make_string(string &value) const {
  ostringstream strm;
  strm << _value;
  value = strm.str();
}


////////////////////////////////////////////////////////////////////
//     Function: P3DFloatValue::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DFloatValue::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "float");
  xvalue->SetDoubleAttribute("value", _value);
  return xvalue;
}
