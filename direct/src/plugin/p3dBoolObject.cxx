// Filename: p3dBoolObject.cxx
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

#include "p3dBoolObject.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DBoolObject::
P3DBoolObject(bool value) : _value(value) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolObject::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DBoolObject::
P3DBoolObject(const P3DBoolObject &copy) :
  P3DObject(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DBoolObject::
get_type() {
  return P3D_OT_bool;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DBoolObject::
get_bool() {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolObject::get_int
//       Access: Public, Virtual
//  Description: Returns the object value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DBoolObject::
get_int() {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DBoolObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DBoolObject::
make_string(string &value) {
  if (_value) {
    value = "True";
  } else {
    value = "False";
  }
}
