// Filename: mouseData.cxx
// Created by:  drose (08Feb99)
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

#include "mouseData.h"

////////////////////////////////////////////////////////////////////
//     Function: MouseData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MouseData::
MouseData() {
  _in_window = false;
  _xpos = 0;
  _ypos = 0;
}
