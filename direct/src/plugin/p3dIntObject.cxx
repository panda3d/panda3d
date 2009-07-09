// Filename: p3dIntObject.cxx
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

#include "p3dIntObject.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DIntObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DIntObject::
P3DIntObject(int value) : _value(value) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntObject::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DIntObject::
P3DIntObject(const P3DIntObject &copy) :
  P3DObject(copy),
  _value(copy._value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DIntObject::
get_type() {
  return P3D_OT_int;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DIntObject::
get_bool() {
  return (_value != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntObject::get_int
//       Access: Public, Virtual
//  Description: Returns the object value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DIntObject::
get_int() {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DIntObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DIntObject::
make_string(string &value) {
  ostringstream strm;
  strm << _value;
  value = strm.str();
}

