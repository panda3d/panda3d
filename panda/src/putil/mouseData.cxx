// Filename: mouseData.cxx
// Created by:  drose (08Feb99)
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
