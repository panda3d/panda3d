// Filename: winGraphicsWindow.I
// Created by:  drose (20Dec02)
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
//     Function: WinGraphicsWindow::handle_mouse_motion
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void WinGraphicsWindow::
handle_mouse_motion(int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::handle_mouse_exit
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void WinGraphicsWindow::
handle_mouse_exit() {
  // note: 'mouse_motion' is considered the 'entry' event
  _input_devices[0].set_pointer_out_of_window();
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::handle_keypress
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void WinGraphicsWindow::
handle_keypress(ButtonHandle key, int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);
  if (key != ButtonHandle::none()) {
    _input_devices[0].button_down(key);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::handle_keyrelease
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void WinGraphicsWindow::
handle_keyrelease(ButtonHandle key) {
  _input_devices[0].button_up(key);
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::translate_mouse
//       Access: Private
//  Description: Translates the mouse pixel coordinate (either x or y)
//               as returned by the Windows message to the signed
//               number expected by Panda.
////////////////////////////////////////////////////////////////////
INLINE int WinGraphicsWindow::
translate_mouse(int pos) const {
  if (pos & 0x8000) {
    pos -= 0x10000;
  }
  return pos;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::set_cursor_in_window
//       Access: Private
//  Description: Called during the window thread in response to the
//               various Windows messages to indicate whether we
//               believe the mouse is presently within the window's
//               client rectangle or not.  This in turn will determine
//               whether we should call update_cursor_window() to hide
//               or show the cursor (or otherwise change its
//               properties) as it moves between the various
//               GraphicsWindows that we control.
////////////////////////////////////////////////////////////////////
INLINE void WinGraphicsWindow::
set_cursor_in_window() {
  if (_cursor_window != this) {
    update_cursor_window(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::set_cursor_out_of_window
//       Access: Private
//  Description: Called during the window thread in response to the
//               various Windows messages to indicate whether we
//               believe the mouse is presently within the window's
//               client rectangle or not.  This in turn will determine
//               whether we should call update_cursor_window() to hide
//               or show the cursor (or otherwise change its
//               properties) as it moves between the various
//               GraphicsWindows that we control.
////////////////////////////////////////////////////////////////////
INLINE void WinGraphicsWindow::
set_cursor_out_of_window() {
  if (_cursor_window == this) {
    update_cursor_window(NULL);
  }
}
