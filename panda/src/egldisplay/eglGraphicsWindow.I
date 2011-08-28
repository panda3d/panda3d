// Filename: eglGraphicsWindow.I
// Created by:  pro-rsoft (21May09)
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


////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::get_xwindow
//       Access: Public
//  Description: Returns the X11 Window handle.
////////////////////////////////////////////////////////////////////
INLINE X11_Window eglGraphicsWindow::
get_xwindow() const {
  return _xwindow;
}
