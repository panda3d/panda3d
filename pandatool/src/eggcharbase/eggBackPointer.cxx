// Filename: eggBackPointer.cxx
// Created by:  drose (26Feb01)
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
//     Function: EggBackPointer::get_frame_rate
//       Access: Public, Virtual
//  Description: Returns the stated frame rate of this particular
//               joint, or 0.0 if it doesn't state.
////////////////////////////////////////////////////////////////////
double EggBackPointer::
get_frame_rate() const {
  return 0.0;
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

////////////////////////////////////////////////////////////////////
//     Function: EggBackPointer::set_name
//       Access: Public, Virtual
//  Description: Applies the indicated name change to the egg file.
////////////////////////////////////////////////////////////////////
void EggBackPointer::
set_name(const string &name) {
}
