/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file x11GraphicsPipe.I
 * @author drose
 * @date 2000-10-30
 */

/**
 * Returns a pointer to the X display associated with the pipe: the display on
 * which to create the windows.
 */
INLINE X11_Display *x11GraphicsPipe::
get_display() const {
  return _display;
}

/**
 * Returns the X screen number associated with the pipe.
 */
INLINE int x11GraphicsPipe::
get_screen() const {
  return _screen;
}

/**
 * Returns the handle to the root window on the pipe's display.
 */
INLINE X11_Window x11GraphicsPipe::
get_root() const {
  return _root;
}

/**
 * Returns the input method opened for the pipe, or NULL if the input method
 * could not be opened for some reason.
 */
INLINE XIM x11GraphicsPipe::
get_im() const {
  return _im;
}

/**
 * Returns an invisible Cursor suitable for assigning to windows that have the
 * cursor_hidden property set.
 */
INLINE X11_Cursor x11GraphicsPipe::
get_hidden_cursor() {
  if (_hidden_cursor == None) {
    make_hidden_cursor();
  }
  return _hidden_cursor;
}

/**
 * Returns true if relative mouse mode is supported on this display.
 */
INLINE bool x11GraphicsPipe::
supports_relative_mouse() const {
  return (_XF86DGADirectVideo != nullptr);
}

/**
 * Enables relative mouse mode for this display.  Returns false if unsupported.
 */
INLINE bool x11GraphicsPipe::
enable_relative_mouse() {
  if (_XF86DGADirectVideo != nullptr) {
    x11display_cat.info() << "Enabling relative mouse using XF86DGA extension\n";
    _XF86DGADirectVideo(_display, _screen, XF86DGADirectMouse);
    return true;
  }
  return false;
}

/**
 * Disables relative mouse mode for this display.
 */
INLINE void x11GraphicsPipe::
disable_relative_mouse() {
  if (_XF86DGADirectVideo != nullptr) {
    x11display_cat.info() << "Disabling relative mouse using XF86DGA extension\n";
    _XF86DGADirectVideo(_display, _screen, 0);
  }
}

/**
 * Globally disables the printing of error messages that are raised by the X11
 * system, for instance in order to test whether a particular X11 operation
 * will succeed.  Reenable error messages with a later call to
 * enable_x_error_messages().
 *
 * The return value is the current value of get_x_error_count().
 */
INLINE int x11GraphicsPipe::
disable_x_error_messages() {
  _x_error_messages_enabled = false;
  return _x_error_count;
}

/**
 * Reenables the printing of error messages after a previous call to
 * disable_x_error_messages().
 *
 * The return value is the current value of get_x_error_count().
 */
INLINE int x11GraphicsPipe::
enable_x_error_messages() {
  _x_error_messages_enabled = true;
  return _x_error_count;
}

/**
 * Returns the number of times an error indication has been raised by the X11
 * system since application start, including errors raised while error
 * messages were disabled.
 */
INLINE int x11GraphicsPipe::
get_x_error_count() {
  return _x_error_count;
}
