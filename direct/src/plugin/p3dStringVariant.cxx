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
P3DStringVariant(const char *value, int length) : 
  P3DVariant(P3D_VT_string),
  _value(value, length)
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

