// Filename: mouseData.I
// Created by:  drose (15Jul02)
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
//     Function: MouseData::get_x
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int MouseData::
get_x() const {
  return _xpos;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseData::get_y
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int MouseData::
get_y() const {
  return _ypos;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseData::get_in_window
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool MouseData::
get_in_window() const {
  return _in_window;
}
