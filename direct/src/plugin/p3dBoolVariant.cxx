// Filename: p3dBoolVariant.cxx
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

#include "p3dBoolVariant.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolVariant::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DBoolVariant::
P3DBoolVariant(bool value) : 
  P3DVariant(P3D_VT_bool),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolVariant::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DBoolVariant::
P3DBoolVariant(const P3DBoolVariant &copy) :
  P3DVariant(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolVariant::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DVariant *P3DBoolVariant::
make_copy() {
  return new P3DBoolVariant(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolVariant::get_bool
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DBoolVariant::
get_bool() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolVariant::get_int
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DBoolVariant::
get_int() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolVariant::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DBoolVariant::
make_string(string &value) const {
  if (_value) {
    value = "1";
  } else {
    value = "0";
  }
}

