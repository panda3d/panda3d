// Filename: charPlacement.cxx
// Created by:  drose (16Feb01)
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

#include "charPlacement.h"
#include "charBitmap.h"


////////////////////////////////////////////////////////////////////
//     Function: CharPlacement::intersects
//       Access: Public
//  Description: Returns true if the particular position this char
//               has been assigned to overlaps the rectangle whose
//               top left corner is at x, y and whose size is given by
//               x_size, y_size, or false otherwise.
////////////////////////////////////////////////////////////////////
bool CharPlacement::
intersects(int x, int y, int x_size, int y_size) const {
  int hright = x + x_size;
  int hbot = y + y_size;

  int mright = _x + _width;
  int mbot = _y + _height;

  return !(x >= mright || hright <= _x ||
           y >= mbot || hbot <= _y);
}
