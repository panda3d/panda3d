// Filename: eggBackPointer.cxx
// Created by:  drose (26Feb01)
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
//     Function: EggBackPointer::extend_to
//       Access: Public, Virtual
//  Description: Extends the table to the indicated number of frames.
////////////////////////////////////////////////////////////////////
void EggBackPointer::
extend_to(int num_frames) {
  // Whoops, can't extend this kind of table!
  nassertv(false);
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
