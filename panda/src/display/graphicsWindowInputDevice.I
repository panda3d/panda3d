// Filename: graphicsWindowInputDevice.I
// Created by:  drose (24May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindowInputDevice::
GraphicsWindowInputDevice() {
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE string GraphicsWindowInputDevice::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::has_pointer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
has_pointer() const {
  return ((_flags & IDF_has_pointer) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::has_keyboard
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
has_keyboard() const {
  return ((_flags & IDF_has_keyboard) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_pointer
//       Access: Public
//  Description: Returns the MouseData associated with the input
//               device's pointer.
////////////////////////////////////////////////////////////////////
INLINE const MouseData &GraphicsWindowInputDevice::
get_pointer() const {
  return _mouse_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_in_window
//       Access: Public
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is within the window, at
//               the given pixel coordinates.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_in_window(int x, int y) {
  _mouse_data._in_window = true;
  _mouse_data._xpos = x;
  _mouse_data._ypos = y;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_out_of_window
//       Access: Public
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is no longer within the
//               window.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_out_of_window() {
  _mouse_data._in_window = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
operator == (const GraphicsWindowInputDevice &) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
operator != (const GraphicsWindowInputDevice &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
operator < (const GraphicsWindowInputDevice &) const {
  return false;
}
