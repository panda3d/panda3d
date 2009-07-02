// Filename: p3dStringObject.cxx
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

#include "p3dStringObject.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DStringObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringObject::
P3DStringObject(const string &value) : 
  P3DObject(P3D_OT_string),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringObject::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringObject::
P3DStringObject(const P3DStringObject &copy) :
  P3DObject(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringObject::
~P3DStringObject() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringObject::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DObject *P3DStringObject::
make_copy() const {
  return new P3DStringObject(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DStringObject::
get_bool() const {
  return !_value.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DStringObject::
make_string(string &value) const {
  value = _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringObject::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DStringObject::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "string");
  xvalue->SetAttribute("value", _value);
  return xvalue;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringObject::output
//       Access: Public, Virtual
//  Description: Writes a formatted representation of the value to the
//               indicated string.  This is intended for developer
//               assistance.
////////////////////////////////////////////////////////////////////
void P3DStringObject::
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
