// Filename: charPlacement.I
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


////////////////////////////////////////////////////////////////////
//     Function: CharPlacement::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CharPlacement::
CharPlacement(const CharBitmap *bm, int x, int y,
              int width, int height) :
  _bm(bm),
  _x(x),
  _y(y),
  _width(width),
  _height(height)
{
}
