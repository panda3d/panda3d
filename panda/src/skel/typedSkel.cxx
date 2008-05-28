// Filename: typedSkel.cxx
// Created by:  jyelon (31Jan07)
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

#include "typedSkel.h"

TypeHandle TypedSkel::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TypedSkel::set_value_alt
//       Access: Public
//  Description: Stores an integer value.  Exact same functionality
//               as set_value, except that this isn't an inline
//               function.
////////////////////////////////////////////////////////////////////
void TypedSkel::
set_value_alt(int n) {
  _value = n;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedSkel::get_value
//       Access: Public
//  Description: Retreives a value that was previously stored.
//               Exact same functionality as get_value, except
//               that this isn't an inline function.
////////////////////////////////////////////////////////////////////
int TypedSkel::
get_value_alt() {
  return _value;
}
