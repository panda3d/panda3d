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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::add_input_device
//       Access: Protected
//  Description: Adds a GraphicsWindowInputDevice to the vector.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
add_input_device(const GraphicsWindowInputDevice &device) {
  _input_devices.push_back(device);
  _input_devices.back().set_device_index(_input_devices.size()-1);
}

