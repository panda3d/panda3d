// Filename: p3dNoneObject.cxx
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

#include "p3dNoneObject.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DNoneObject::
P3DNoneObject() : P3DObject(P3D_OT_none) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneObject::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DObject *P3DNoneObject::
make_copy() const {
  return new P3DNoneObject(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DNoneObject::
get_bool() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DNoneObject::
make_string(string &value) const {
  value = "None";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneObject::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DNoneObject::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "none");
  return xvalue;
}
