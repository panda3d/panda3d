// Filename: charBitmap.I
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
//     Function: CharBitmap::get_width
//       Access: Public
//  Description: Returns the width of the character in pixels.
////////////////////////////////////////////////////////////////////
INLINE int CharBitmap::
get_width() const {
  return _block.empty() ? 0 : _block[0].size();
}

////////////////////////////////////////////////////////////////////
//     Function: CharBitmap::get_height
//       Access: Public
//  Description: Returns the height of the character in pixels.
////////////////////////////////////////////////////////////////////
INLINE int CharBitmap::
get_height() const {
  return _block.size();
}
