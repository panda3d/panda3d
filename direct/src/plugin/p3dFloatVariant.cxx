// Filename: p3dFloatVariant.cxx
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

#include "p3dFloatVariant.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatVariant::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFloatVariant::
P3DFloatVariant(double value) : 
  P3DVariant(P3D_VT_float),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatVariant::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFloatVariant::
P3DFloatVariant(const P3DFloatVariant &copy) :
  P3DVariant(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatVariant::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DVariant *P3DFloatVariant::
make_copy() {
  return new P3DFloatVariant(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatVariant::get_bool
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DFloatVariant::
get_bool() const {
  return (_value != 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatVariant::get_int
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DFloatVariant::
get_int() const {
  return (int)_value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatVariant::get_float
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to a floating-point
//               value, if possible.
////////////////////////////////////////////////////////////////////
double P3DFloatVariant::
get_float() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatVariant::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DFloatVariant::
make_string(string &value) const {
  ostringstream strm;
  strm << _value;
  value = strm.str();
}


////////////////////////////////////////////////////////////////////
//     Function: P3DFloatVariant::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this variant.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DFloatVariant::
make_xml() const {
  TiXmlElement *xvariant = new TiXmlElement("variant");
  xvariant->SetAttribute("type", "float");
  xvariant->SetDoubleAttribute("value", _value);
  return xvariant;
}
