// Filename: glxDisplay.I
// Created by:  drose (30Oct00)
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
//     Function: glxDisplay::get_display
//       Access: Public
//  Description: Returns a pointer to the X display associated with
//               the pipe: the display on which to create the windows.
////////////////////////////////////////////////////////////////////
INLINE Display *glxDisplay::
get_display() const {
  return _display;
}

////////////////////////////////////////////////////////////////////
//     Function: glxDisplay::get_screen
//       Access: Public
//  Description: Returns the X screen number associated with the pipe.
////////////////////////////////////////////////////////////////////
INLINE int glxDisplay::
get_screen() const {
  return _screen;
}

////////////////////////////////////////////////////////////////////
//     Function: glxDisplay::get_root
//       Access: Public
//  Description: Returns the handle to the root window on the pipe's
//               display.
////////////////////////////////////////////////////////////////////
INLINE Window glxDisplay::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: glxDisplay::get_display_width
//       Access: Public
//  Description: Returns the width of the entire display.
////////////////////////////////////////////////////////////////////
INLINE int glxDisplay::
get_display_width() const {
  return _width;
}

////////////////////////////////////////////////////////////////////
//     Function: glxDisplay::get_display_height
//       Access: Public
//  Description: Returns the height of the entire display.
////////////////////////////////////////////////////////////////////
INLINE int glxDisplay::
get_display_height() const {
  return _height;
}
