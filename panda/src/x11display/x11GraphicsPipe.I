// Filename: x11GraphicsPipe.I
// Created by:  drose (30Oct00)
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
//     Function: x11GraphicsPipe::get_display
//       Access: Public
//  Description: Returns a pointer to the X display associated with
//               the pipe: the display on which to create the windows.
////////////////////////////////////////////////////////////////////
INLINE Display *x11GraphicsPipe::
get_display() const {
  return _display;
}

////////////////////////////////////////////////////////////////////
//     Function: x11GraphicsPipe::get_screen
//       Access: Public
//  Description: Returns the X screen number associated with the pipe.
////////////////////////////////////////////////////////////////////
INLINE int x11GraphicsPipe::
get_screen() const {
  return _screen;
}

////////////////////////////////////////////////////////////////////
//     Function: x11GraphicsPipe::get_root
//       Access: Public
//  Description: Returns the handle to the root window on the pipe's
//               display.
////////////////////////////////////////////////////////////////////
INLINE Window x11GraphicsPipe::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: x11GraphicsPipe::get_im
//       Access: Public
//  Description: Returns the input method opened for the pipe, or NULL
//               if the input method could not be opened for some
//               reason.
////////////////////////////////////////////////////////////////////
INLINE XIM x11GraphicsPipe::
get_im() const {
  return _im;
}

////////////////////////////////////////////////////////////////////
//     Function: x11GraphicsPipe::get_hidden_cursor
//       Access: Public
//  Description: Returns an invisible Cursor suitable for assigning to
//               windows that have the cursor_hidden property set.
////////////////////////////////////////////////////////////////////
INLINE Cursor x11GraphicsPipe::
get_hidden_cursor() {
  if (_hidden_cursor == None) {
    make_hidden_cursor();
  }
  return _hidden_cursor;
}

////////////////////////////////////////////////////////////////////
//     Function: x11GraphicsPipe::x11Handle::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE x11GraphicsPipe::x11Handle::
x11Handle(Window handle) : _handle(handle) {
}

////////////////////////////////////////////////////////////////////
//     Function: x11GraphicsPipe::x11Handle::get_handle
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Window x11GraphicsPipe::x11Handle::
get_handle() const {
  return _handle;
}
