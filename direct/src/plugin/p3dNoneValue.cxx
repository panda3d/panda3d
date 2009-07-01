// Filename: p3dNoneValue.cxx
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

#include "p3dNoneValue.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneValue::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DNoneValue::
P3DNoneValue() : P3DValue(P3D_VT_none) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneValue::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DValue *P3DNoneValue::
make_copy() {
  return new P3DNoneValue(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneValue::get_bool
//       Access: Public, Virtual
//  Description: Returns the value value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DNoneValue::
get_bool() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneValue::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DNoneValue::
make_string(string &value) const {
  value = "None";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneValue::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DNoneValue::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "none");
  return xvalue;
}
