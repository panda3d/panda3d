// Filename: tinyXGraphicsPipe.I
// Created by:  drose (03May08)
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
//     Function: TinyXGraphicsPipe::get_display
//       Access: Public
//  Description: Returns a pointer to the X display associated with
//               the pipe: the display on which to create the windows.
////////////////////////////////////////////////////////////////////
INLINE Display *TinyXGraphicsPipe::
get_display() const {
  return _display;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyXGraphicsPipe::get_screen
//       Access: Public
//  Description: Returns the X screen number associated with the pipe.
////////////////////////////////////////////////////////////////////
INLINE int TinyXGraphicsPipe::
get_screen() const {
  return _screen;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyXGraphicsPipe::get_root
//       Access: Public
//  Description: Returns the handle to the root window on the pipe's
//               display.
////////////////////////////////////////////////////////////////////
INLINE Window TinyXGraphicsPipe::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyXGraphicsPipe::get_im
//       Access: Public
//  Description: Returns the input method opened for the pipe, or NULL
//               if the input method could not be opened for some
//               reason.
////////////////////////////////////////////////////////////////////
INLINE XIM TinyXGraphicsPipe::
get_im() const {
  return _im;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyXGraphicsPipe::get_hidden_cursor
//       Access: Public
//  Description: Returns an invisible Cursor suitable for assigning to
//               windows that have the cursor_hidden property set.
////////////////////////////////////////////////////////////////////
INLINE Cursor TinyXGraphicsPipe::
get_hidden_cursor() {
  if (_hidden_cursor == None) {
    make_hidden_cursor();
  }
  return _hidden_cursor;
}
