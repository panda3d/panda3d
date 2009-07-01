// Filename: p3dStringVariant.cxx
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

#include "p3dStringVariant.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DStringVariant::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringVariant::
P3DStringVariant(const string &value) : 
  P3DVariant(P3D_VT_string),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringVariant::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringVariant::
P3DStringVariant(const P3DStringVariant &copy) :
  P3DVariant(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringVariant::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DStringVariant::
~P3DStringVariant() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringVariant::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DVariant *P3DStringVariant::
make_copy() {
  return new P3DStringVariant(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringVariant::get_bool
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DStringVariant::
get_bool() const {
  return !_value.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringVariant::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DStringVariant::
make_string(string &value) const {
  value = _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringVariant::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this variant.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DStringVariant::
make_xml() const {
  TiXmlElement *xvariant = new TiXmlElement("variant");
  xvariant->SetAttribute("type", "string");
  xvariant->SetAttribute("value", _value);
  return xvariant;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DStringVariant::output
//       Access: Public, Virtual
//  Description: Writes a formatted representation of the value to the
//               indicated string.  This is intended for developer
//               assistance.
////////////////////////////////////////////////////////////////////
void P3DStringVariant::
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
