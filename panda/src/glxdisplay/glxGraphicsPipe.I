// Filename: glxGraphicsPipe.I
// Created by:  drose (30Oct00)
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
//     Function: glxGraphicsPipe::get_display
//       Access: Public
//  Description: Returns a pointer to the X display associated with
//               the pipe: the display on which to create the windows.
////////////////////////////////////////////////////////////////////
INLINE Display *glxGraphicsPipe::
get_display() const {
  return _display;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_screen
//       Access: Public
//  Description: Returns the X screen number associated with the pipe.
////////////////////////////////////////////////////////////////////
INLINE int glxGraphicsPipe::
get_screen() const {
  return _screen;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_root
//       Access: Public
//  Description: Returns the handle to the root window on the pipe's
//               display.
////////////////////////////////////////////////////////////////////
INLINE Window glxGraphicsPipe::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_wm_delete_window
//       Access: Public
//  Description: Returns the X atom that represents WM_DELETE_WINDOW
//               to the current display.
////////////////////////////////////////////////////////////////////
INLINE Atom glxGraphicsPipe::
get_wm_delete_window() const {
  return _wm_delete_window;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_im
//       Access: Public
//  Description: Returns the input method opened for the pipe, or NULL
//               if the input method could not be opened for some
//               reason.
////////////////////////////////////////////////////////////////////
INLINE XIM glxGraphicsPipe::
get_im() const {
  return _im;
}
