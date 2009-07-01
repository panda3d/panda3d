// Filename: p3dBoolValue.cxx
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

#include "p3dBoolValue.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolValue::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DBoolValue::
P3DBoolValue(bool value) : 
  P3DValue(P3D_VT_bool),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolValue::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DBoolValue::
P3DBoolValue(const P3DBoolValue &copy) :
  P3DValue(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolValue::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DValue *P3DBoolValue::
make_copy() {
  return new P3DBoolValue(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolValue::get_bool
//       Access: Public, Virtual
//  Description: Returns the value value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DBoolValue::
get_bool() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolValue::get_int
//       Access: Public, Virtual
//  Description: Returns the value value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DBoolValue::
get_int() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolValue::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DBoolValue::
make_string(string &value) const {
  if (_value) {
    value = "True";
  } else {
    value = "False";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolValue::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DBoolValue::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "bool");
  xvalue->SetAttribute("value", (int)_value);
  return xvalue;
}
