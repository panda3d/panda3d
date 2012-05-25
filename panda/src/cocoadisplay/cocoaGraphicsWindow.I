// Filename: cocoaGraphicsWindow.I
// Created by:  rdb (14May12)
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
//     Function: CocoaGraphicsWindow::get_nswindow
//       Access: Public
//  Description: Returns a pointer to the underlying NSWindow.
////////////////////////////////////////////////////////////////////
INLINE NSWindow *CocoaGraphicsWindow::
get_nswindow() const {
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::get_nsview
//       Access: Public
//  Description: Returns a pointer to the underlying NSView.
////////////////////////////////////////////////////////////////////
INLINE NSView *CocoaGraphicsWindow::
get_nsview() const {
  return _view;
}
