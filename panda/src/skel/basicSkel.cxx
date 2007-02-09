// Filename: basicSkel.cxx
// Created by:  jyelon (31Jan07)
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

#include "basicSkel.h"

////////////////////////////////////////////////////////////////////
//     Function: BasicSkel::set_value_alt
//       Access: Public
//  Description: Stores an integer value.  Exact same functionality
//               as set_value, except that this isn't an inline
//               function.
////////////////////////////////////////////////////////////////////
void BasicSkel::
set_value_alt(int n) {
  _value = n;
}

////////////////////////////////////////////////////////////////////
//     Function: BasicSkel::get_value
//       Access: Public
//  Description: Retreives a value that was previously stored.
//               Exact same functionality as get_value, except
//               that this isn't an inline function.
////////////////////////////////////////////////////////////////////
int BasicSkel::
get_value_alt() {
  return _value;
}
