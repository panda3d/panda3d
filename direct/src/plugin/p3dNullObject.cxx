// Filename: p3dNullObject.cxx
// Created by:  drose (07Jul09)
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

#include "p3dNullObject.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DNullObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DNullObject::
P3DNullObject() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNullObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DNullObject::
get_type() const {
  return P3D_OT_null;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNullObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DNullObject::
get_bool() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNullObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DNullObject::
make_string(string &value) const {
  value = "Null";
}
