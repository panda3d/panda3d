// Filename: p3dIntVariant.cxx
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

#include "p3dIntVariant.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DIntVariant::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DIntVariant::
P3DIntVariant(int value) : 
  P3DVariant(P3D_VT_int),
  _value(value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntVariant::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DIntVariant::
P3DIntVariant(const P3DIntVariant &copy) :
  P3DVariant(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntVariant::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DVariant *P3DIntVariant::
make_copy() {
  return new P3DIntVariant(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntVariant::get_bool
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DIntVariant::
get_bool() const {
  return (_value != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntVariant::get_int
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DIntVariant::
get_int() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntVariant::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DIntVariant::
make_string(string &value) const {
  ostringstream strm;
  strm << _value;
  value = strm.str();
}

