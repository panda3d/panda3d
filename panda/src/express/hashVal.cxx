// Filename: hashVal.cxx
// Created by:  drose (14Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "hashVal.h"

////////////////////////////////////////////////////////////////////
//     Function: HashVal::as_string
//       Access: Published
//  Description: Returns the HashVal as a string with four numbers.
////////////////////////////////////////////////////////////////////
string HashVal::
as_string() const {
  ostringstream strm;
  output(strm);
  return strm.str();
}
