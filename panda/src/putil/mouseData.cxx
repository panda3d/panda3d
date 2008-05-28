// Filename: mouseData.cxx
// Created by:  drose (08Feb99)
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

#include "mouseData.h"

////////////////////////////////////////////////////////////////////
//     Function: MouseData::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void MouseData::
output(ostream &out) const {
  if (!_in_window) {
    out << "MouseData: Not in window";
  } else {
    out << "MouseData: (" << _xpos << ", " << _ypos << ")";
  }
}
