// Filename: xFileDataObject.cxx
// Created by:  drose (03Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "xFileDataObject.h"

TypeHandle XFileDataObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::as_integer_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as an integer, if
//               it has one.
////////////////////////////////////////////////////////////////////
int XFileDataObject::
as_integer_value() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::as_double_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a double, if
//               it has one.
////////////////////////////////////////////////////////////////////
double XFileDataObject::
as_double_value() const {
  return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::as_string_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a string, if
//               it has one.
////////////////////////////////////////////////////////////////////
string XFileDataObject::
as_string_value() const {
  return string();
}
