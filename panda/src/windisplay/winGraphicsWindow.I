/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winGraphicsWindow.I
 * @author drose
 * @date 2002-12-20
 */

/**
 * Translates the mouse pixel coordinate (either x or y) as returned by the
 * Windows message to the signed number expected by Panda.
 */
INLINE int WinGraphicsWindow::
translate_mouse(int pos) const {
  if (pos & 0x8000) {
    pos -= 0x10000;
  }
  return pos;
}

/**
 * Called during the window thread in response to the various Windows messages
 * to indicate whether we believe the mouse is presently within the window's
 * client rectangle or not.  This in turn will determine whether we should
 * call update_cursor_window() to hide or show the cursor (or otherwise change
 * its properties) as it moves between the various GraphicsWindows that we
 * control.
 */
INLINE void WinGraphicsWindow::
set_cursor_in_window() {
  if (_cursor_window != this) {
    update_cursor_window(this);
  }
}

/**
 * Called during the window thread in response to the various Windows messages
 * to indicate whether we believe the mouse is presently within the window's
 * client rectangle or not.  This in turn will determine whether we should
 * call update_cursor_window() to hide or show the cursor (or otherwise change
 * its properties) as it moves between the various GraphicsWindows that we
 * control.
 */
INLINE void WinGraphicsWindow::
set_cursor_out_of_window() {
  if (_cursor_window == this) {
    update_cursor_window(nullptr);
  }
}

/**
 * May be called only during the servicing of a Windows message.  This returns
 * the time the message was added to the Windows message queue (as reported
 * via GetMessageTime()), converted into global clock units.
 */
INLINE double WinGraphicsWindow::
get_message_time() {
  DWORD now_ticks = GetTickCount();
  double now_time = ClockObject::get_global_clock()->get_real_time();
  DWORD elapsed_ticks = now_ticks - GetMessageTime();
  return now_time - (double)elapsed_ticks / 1000.0;
}

/**
 * Return the IME_window handle if open
 */
INLINE HWND WinGraphicsWindow::
get_ime_hwnd() {
  if (_ime_active)
    return _ime_hWnd;
  else
    return nullptr;
}

/**
 *
 */
INLINE WinGraphicsWindow::WindowClass::
WindowClass(const WindowProperties &props) :
  _icon(0)
{
  if (props.has_icon_filename()) {
    _icon = get_icon(props.get_icon_filename());
  }
}

/**
 *
 */
INLINE bool WinGraphicsWindow::WindowClass::
operator < (const WinGraphicsWindow::WindowClass &other) const {
  return _icon < other._icon;
}
