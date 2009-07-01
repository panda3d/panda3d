// Filename: p3dIntValue.cxx
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

#include "p3dIntValue.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DIntValue::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DIntValue::
P3DIntValue(int value) : 
  P3DValue(P3D_VT_int),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntValue::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DIntValue::
P3DIntValue(const P3DIntValue &copy) :
  P3DValue(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntValue::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DValue *P3DIntValue::
make_copy() {
  return new P3DIntValue(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntValue::get_bool
//       Access: Public, Virtual
//  Description: Returns the value value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DIntValue::
get_bool() const {
  return (_value != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntValue::get_int
//       Access: Public, Virtual
//  Description: Returns the value value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DIntValue::
get_int() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntValue::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DIntValue::
make_string(string &value) const {
  ostringstream strm;
  strm << _value;
  value = strm.str();
}


////////////////////////////////////////////////////////////////////
//     Function: P3DIntValue::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DIntValue::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "int");
  xvalue->SetAttribute("value", _value);
  return xvalue;
}
