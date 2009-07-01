// Filename: p3dStringValue.cxx
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

#include "p3dStringValue.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DStringValue::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringValue::
P3DStringValue(const string &value) : 
  P3DValue(P3D_VT_string),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringValue::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringValue::
P3DStringValue(const P3DStringValue &copy) :
  P3DValue(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringValue::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringValue::
~P3DStringValue() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringValue::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DValue *P3DStringValue::
make_copy() {
  return new P3DStringValue(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringValue::get_bool
//       Access: Public, Virtual
//  Description: Returns the value value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DStringValue::
get_bool() const {
  return !_value.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringValue::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DStringValue::
make_string(string &value) const {
  value = _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringValue::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DStringValue::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "string");
  xvalue->SetAttribute("value", _value);
  return xvalue;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringValue::output
//       Access: Public, Virtual
//  Description: Writes a formatted representation of the value to the
//               indicated string.  This is intended for developer
//               assistance.
////////////////////////////////////////////////////////////////////
void P3DStringValue::
output(ostream &out) const {
  out << '"';
  for (string::const_iterator si = _value.begin(); si != _value.end(); ++si) {
    if (isprint(*si)) {
      switch (*si) {
      case '"':
        out << "\\\x22";
        break;

      default:
        out << *si;
      }
    } else {
      switch (*si) {
      case '\n':
        out << "\\n";
        break;

      case '\t':
        out << "\\t";
        break;

      default:
        {
          char buffer[128];
          sprintf(buffer, "%02x", (unsigned char)(*si));
          out << "\\x" << buffer;
        }
      }
    }
  }
  out << '"';
}
