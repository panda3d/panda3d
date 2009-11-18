// Filename: p3dWindowParams.I
// Created by:  drose (22Jun09)
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
//     Function: P3DWindowParams::get_window_type
//       Access: Public
//  Description: Returns the window_type that was passed to the
//               constructor, or to set_window_type().
////////////////////////////////////////////////////////////////////
inline P3D_window_type P3DWindowParams::
get_window_type() const {
  return _window_type;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWindowParams::set_window_type
//       Access: Public
//  Description: Changes the window_type.
////////////////////////////////////////////////////////////////////
inline void P3DWindowParams::
set_window_type(P3D_window_type window_type) {
  _window_type = window_type;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWindowParams::get_window_x
//       Access: Public
//  Description: Returns the window origin X coordinate that was
//               passed to the constructor.
////////////////////////////////////////////////////////////////////
inline int P3DWindowParams::
get_win_x() const {
  return _win_x;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWindowParams::get_win_y
//       Access: Public
//  Description: Returns the window origin Y coordinate that was
//               passed to the constructor.
////////////////////////////////////////////////////////////////////
inline int P3DWindowParams::
get_win_y() const {
  return _win_y;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWindowParams::get_win_width
//       Access: Public
//  Description: Returns the window width that was passed to the
//               constructor.
////////////////////////////////////////////////////////////////////
inline int P3DWindowParams::
get_win_width() const {
  return _win_width;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWindowParams::get_win_height
//       Access: Public
//  Description: Returns the window height that was passed to the
//               constructor.
////////////////////////////////////////////////////////////////////
inline int P3DWindowParams::
get_win_height() const {
  return _win_height;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWindowParams::get_parent_window
//       Access: Public
//  Description: Returns the parent window handle that was passed to
//               the constructor.
////////////////////////////////////////////////////////////////////
inline const P3D_window_handle &P3DWindowParams::
get_parent_window() const {
  return _parent_window;
}
