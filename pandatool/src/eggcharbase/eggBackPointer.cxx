// Filename: eggBackPointer.cxx
// Created by:  drose (26Feb01)
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

#include "eggBackPointer.h"


TypeHandle EggBackPointer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggBackPointer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggBackPointer::
EggBackPointer() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggBackPointer::has_vertices
//       Access: Public, Virtual
//  Description: Returns true if there are any vertices referenced by
//               the node this points to, false otherwise.  For
//               certain kinds of back pointers (e.g. table animation
//               entries), this is always false.
////////////////////////////////////////////////////////////////////
bool EggBackPointer::
has_vertices() const {
  return false;
}
