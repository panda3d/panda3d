// Filename: p3dValue.cxx
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

#include "p3dValue.h"
#include <string.h>  // memcpy

////////////////////////////////////////////////////////////////////
//     Function: P3DValue::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DValue::
~P3DValue() {
  _type = P3D_VT_none;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DValue::get_int
//       Access: Public, Virtual
//  Description: Returns the value value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DValue::
get_int() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DValue::get_float
//       Access: Public, Virtual
//  Description: Returns the value value coerced to a floating-point
//               value, if possible.
////////////////////////////////////////////////////////////////////
double P3DValue::
get_float() const {
  return get_int();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DValue::get_string_length
//       Access: Public, Virtual
//  Description: Returns the length of the string that represents the
//               value value, not counting any null termination
//               characters.
////////////////////////////////////////////////////////////////////
int P3DValue::
get_string_length() const {
  string result;
  make_string(result);
  return result.length();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DValue::extract_string
//       Access: Public, Virtual
//  Description: Stores a string that represents the value value in
//               the indicated buffer; a null character is included if
//               there is space.  Returns the number of characters
//               needed in the output (which might be more than the
//               actual number of characters stored if buffer_length
//               was too small).
////////////////////////////////////////////////////////////////////
int P3DValue::
extract_string(char *buffer, int buffer_length) const {
  string result;
  make_string(result);
  memcpy(buffer, result.c_str(), min(buffer_length, (int)result.size() + 1));
  return (int)result.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DValue::get_list_length
//       Access: Public, Virtual
//  Description: Returns the length of the value value as a list.
////////////////////////////////////////////////////////////////////
int P3DValue::
get_list_length() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DValue::get_list_item
//       Access: Public, Virtual
//  Description: Returns the nth item in the value as a list.  The
//               return value is a freshly-allocated P3DValue object
//               that must be deleted by the caller, or NULL on error.
////////////////////////////////////////////////////////////////////
P3DValue *P3DValue::
get_list_item(int n) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DValue::output
//       Access: Public, Virtual
//  Description: Writes a formatted representation of the value to the
//               indicated string.  This is intended for developer
//               assistance.
////////////////////////////////////////////////////////////////////
void P3DValue::
output(ostream &out) const {
  string value;
  make_string(value);
  out << value;
}
