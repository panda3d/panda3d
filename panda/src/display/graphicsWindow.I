// Filename: graphicsWindow.I
// Created by:  frang (07Mar99)
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
//     Function: GraphicsWindow::is_closed
//       Access: Published
//  Description: Returns true if the window has not yet been opened,
//               or has been fully closed, false if it is open.  The
//               window is not opened immediately after
//               GraphicsEngine::make_window() is called; nor is it
//               closed immediately after
//               GraphicsEngine::remove_window() is called.  Either
//               operation may take a frame or two.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
is_closed() const {
  return !_properties.get_open();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::is_fullscreen
//       Access: Published
//  Description: Returns true if the window has been opened as a
//               fullscreen window, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
is_fullscreen() const {
  return _properties.get_fullscreen();
}
