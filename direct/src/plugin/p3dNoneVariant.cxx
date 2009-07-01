// Filename: p3dNoneVariant.cxx
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

#include "p3dNoneVariant.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneVariant::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DNoneVariant::
P3DNoneVariant() : P3DVariant(P3D_VT_none) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneVariant::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DVariant *P3DNoneVariant::
make_copy() {
  return new P3DNoneVariant(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneVariant::get_bool
//       Access: Public, Virtual
//  Description: Returns the variant value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DNoneVariant::
get_bool() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DNoneVariant::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DNoneVariant::
make_string(string &value) const {
  value = "None";
}
