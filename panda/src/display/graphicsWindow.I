// Filename: graphicsWindow.I
// Created by:  frang (07Mar99)
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_unexposed_draw
//       Access: Published
//  Description: If this flag is false, the window is redrawn only
//               after it has received a recent "unexpose" or "draw"
//               event from the underlying windowing systme.  If this
//               flag is true, the window is redrawn every frame
//               regardless.  Setting this false may prevent the
//               window from redrawing unnecessarily when it is
//               hidden, and may play nicer with other windows on the
//               desktop, but may adversely affect frame rate even
//               when the window is fully visible; setting it true will
//               ensure that the window contents are always current.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
set_unexposed_draw(bool unexposed_draw) {
  _unexposed_draw = unexposed_draw;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_unexposed_draw
//       Access: Published
//  Description: See set_unexposed_draw().
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
get_unexposed_draw() const {
  return _unexposed_draw;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_window_handle
//       Access: Published
//  Description: Returns the WindowHandle corresponding to this window
//               on the desktop.  This is mainly useful for
//               communicating with external libraries.  Use
//               window_handle->get_os_handle()->get_handle(), or
//               window_handle->get_string_handle(), to get the actual
//               OS-specific window handle object, whatever type that
//               might be.
////////////////////////////////////////////////////////////////////
INLINE WindowHandle *GraphicsWindow::
get_window_handle() const {
  return _window_handle;
}

