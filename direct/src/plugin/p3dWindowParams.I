/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dWindowParams.I
 * @author drose
 * @date 2009-06-22
 */

/**
 * Returns the window_type that was passed to the constructor, or to
 * set_window_type().
 */
inline P3D_window_type P3DWindowParams::
get_window_type() const {
  return _window_type;
}

/**
 * Changes the window_type.
 */
inline void P3DWindowParams::
set_window_type(P3D_window_type window_type) {
  _window_type = window_type;
}

/**
 * Returns the window origin X coordinate that was passed to the constructor.
 */
inline int P3DWindowParams::
get_win_x() const {
  return _win_x;
}

/**
 * Returns the window origin Y coordinate that was passed to the constructor.
 */
inline int P3DWindowParams::
get_win_y() const {
  return _win_y;
}

/**
 * Returns the window width that was passed to the constructor.
 */
inline int P3DWindowParams::
get_win_width() const {
  return _win_width;
}

/**
 * Returns the window height that was passed to the constructor.
 */
inline int P3DWindowParams::
get_win_height() const {
  return _win_height;
}

/**
 * Returns the parent window handle that was passed to the constructor.
 */
inline const P3D_window_handle &P3DWindowParams::
get_parent_window() const {
  return _parent_window;
}
