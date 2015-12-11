// Filename: graphicsWindowInputDevice.I
// Created by:  drose (24May00)
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
//     Function: GraphicsWindowInputDevice::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindowInputDevice::
GraphicsWindowInputDevice() : _host(NULL) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_device_index
//       Access: Public
//  Description: Set the device index.  This is reported in pointer
//               events.  The device index will be equal to the position
//               of the GraphicsWindowInputDevice in the window's list.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_device_index(int index) {
  _device_index = index;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_in_window
//       Access: Published
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is within the window, at
//               the given pixel coordinates.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_in_window(double x, double y, double time) {
  LightMutexHolder holder(_lock);
  InputDevice::set_pointer(true, x, y, time);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_out_of_window
//       Access: Published
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is no longer within the
//               window.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_out_of_window(double time) {
  LightMutexHolder holder(_lock);
  InputDevice::set_pointer_out_of_window(time);
}
