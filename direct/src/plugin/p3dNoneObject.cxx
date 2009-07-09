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
P3DNoneObject() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DNoneObject::
get_type() {
  return P3D_OT_none;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DNoneObject::
get_bool() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DNoneObject::
make_string(string &value) {
  value = "None";
}
