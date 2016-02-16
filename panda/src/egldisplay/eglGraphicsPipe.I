/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsPipe.I
 * @author rdb
 * @date 2009-05-21
 */

/**
 * Returns a pointer to the X display associated with the pipe: the display on
 * which to create the windows.
 */
INLINE X11_Display *eglGraphicsPipe::
get_display() const {
  return _display;
}

/**
 * Returns the X screen number associated with the pipe.
 */
INLINE int eglGraphicsPipe::
get_screen() const {
  return _screen;
}

/**
 * Returns the handle to the root window on the pipe's display.
 */
INLINE X11_Window eglGraphicsPipe::
get_root() const {
  return _root;
}

/**
 * Returns the input method opened for the pipe, or NULL if the input method
 * could not be opened for some reason.
 */
INLINE XIM eglGraphicsPipe::
get_im() const {
  return _im;
}

/**
 * Returns an invisible Cursor suitable for assigning to windows that have the
 * cursor_hidden property set.
 */
INLINE X11_Cursor eglGraphicsPipe::
get_hidden_cursor() {
  if (_hidden_cursor == None) {
    make_hidden_cursor();
  }
  return _hidden_cursor;
}
