// Filename: eggVertexPointer.cxx
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

#include "eggVertexPointer.h"


TypeHandle EggVertexPointer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPointer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexPointer::
EggVertexPointer(EggObject *egg_object) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPointer::get_num_frames
//       Access: Public, Virtual
//  Description: Returns the number of frames of animation for this
//               particular slider.
////////////////////////////////////////////////////////////////////
int EggVertexPointer::
get_num_frames() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPointer::get_frame
//       Access: Public, Virtual
//  Description: Returns the value corresponding to this
//               slider position in the nth frame.
////////////////////////////////////////////////////////////////////
double EggVertexPointer::
get_frame(int n) const {
  nassertr(false, 0.0);
  return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexPointer::has_vertices
//       Access: Public, Virtual
//  Description: Returns true if there are any vertices referenced by
//               the node this points to, false otherwise.  For
//               certain kinds of back pointers (e.g. table animation
//               entries), this is always false.
////////////////////////////////////////////////////////////////////
bool EggVertexPointer::
has_vertices() const {
  return true;
}
