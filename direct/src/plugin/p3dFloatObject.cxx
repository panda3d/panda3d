// Filename: p3dFloatObject.cxx
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

#include "p3dFloatObject.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFloatObject::
P3DFloatObject(double value) : _value(value) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatObject::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DFloatObject::
P3DFloatObject(const P3DFloatObject &copy) :
  P3DObject(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DFloatObject::
get_type() {
  return P3D_OT_float;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DFloatObject::
get_bool() {
  return (_value != 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatObject::get_int
//       Access: Public, Virtual
//  Description: Returns the object value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DFloatObject::
get_int() {
  return (int)_value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatObject::get_float
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a floating-point
//               value, if possible.
////////////////////////////////////////////////////////////////////
double P3DFloatObject::
get_float() {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFloatObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DFloatObject::
make_string(string &value) {
  ostringstream strm;
  strm << _value;
  value = strm.str();
}
