// Filename: eglGraphicsPipe.I
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
//     Function: eglGraphicsPipe::get_display
//       Access: Public
//  Description: Returns a pointer to the X display associated with
//               the pipe: the display on which to create the windows.
////////////////////////////////////////////////////////////////////
INLINE X11_Display *eglGraphicsPipe::
get_display() const {
  return _display;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::get_screen
//       Access: Public
//  Description: Returns the X screen number associated with the pipe.
////////////////////////////////////////////////////////////////////
INLINE int eglGraphicsPipe::
get_screen() const {
  return _screen;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::get_root
//       Access: Public
//  Description: Returns the handle to the root window on the pipe's
//               display.
////////////////////////////////////////////////////////////////////
INLINE X11_Window eglGraphicsPipe::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::get_im
//       Access: Public
//  Description: Returns the input method opened for the pipe, or NULL
//               if the input method could not be opened for some
//               reason.
////////////////////////////////////////////////////////////////////
INLINE XIM eglGraphicsPipe::
get_im() const {
  return _im;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::get_hidden_cursor
//       Access: Public
//  Description: Returns an invisible Cursor suitable for assigning to
//               windows that have the cursor_hidden property set.
////////////////////////////////////////////////////////////////////
INLINE X11_Cursor eglGraphicsPipe::
get_hidden_cursor() {
  if (_hidden_cursor == None) {
    make_hidden_cursor();
  }
  return _hidden_cursor;
}
