// Filename: windowProperties.I
// Created by:  drose (13Aug02)
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
//     Function: WindowProperties::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WindowProperties::
WindowProperties(const WindowProperties &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WindowProperties::
~WindowProperties() {
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
operator != (const WindowProperties &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::is_any_specified
//       Access: Published
//  Description: Returns true if any properties have been specified,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
is_any_specified() const {
  return (_specified != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_origin
//       Access: Published
//  Description: Specifies the origin on the screen (in pixels,
//               relative to the top-left corner) at which the window
//               should appear.  This is the origin of the top-left
//               corner of the useful part of the window, not
//               including decorations.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_origin(int x_origin, int y_origin) {
  _x_origin = x_origin;
  _y_origin = y_origin;
  _specified |= S_origin;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_x_origin
//       Access: Published
//  Description: Returns the x coordinate of the window's top-left
//               corner, not including decorations.
////////////////////////////////////////////////////////////////////
INLINE int WindowProperties::
get_x_origin() const {
  nassertr(has_origin(), 0);
  return _x_origin;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_y_origin
//       Access: Published
//  Description: Returns the y coordinate of the window's top-left
//               corner, not including decorations.
////////////////////////////////////////////////////////////////////
INLINE int WindowProperties::
get_y_origin() const {
  nassertr(has_origin(), 0);
  return _y_origin;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_origin
//       Access: Published
//  Description: Returns true if the window origin has been specified,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_origin() const {
  return ((_specified & S_origin) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_origin
//       Access: Published
//  Description: Removes the origin specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_origin() {
  _specified &= ~S_origin;
  _x_origin = 0;
  _y_origin = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_size
//       Access: Published
//  Description: Specifies the requested size of the window, in
//               pixels.  This is the size of the useful part of the
//               window, not including decorations.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_size(int x_size, int y_size) {
  _x_size = x_size;
  _y_size = y_size;
  _specified |= S_size;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_x_size
//       Access: Published
//  Description: Returns size in pixels in the x dimension of the
//               useful part of the window, not including decorations.
//               That is, this is the window's width.
////////////////////////////////////////////////////////////////////
INLINE int WindowProperties::
get_x_size() const {
  nassertr(has_size(), 0);
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_y_size
//       Access: Published
//  Description: Returns size in pixels in the y dimension of the
//               useful part of the window, not including decorations.
//               That is, this is the window's height.
////////////////////////////////////////////////////////////////////
INLINE int WindowProperties::
get_y_size() const {
  nassertr(has_size(), 0);
  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_size
//       Access: Published
//  Description: Returns true if the window size has been specified,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_size() const {
  return ((_specified & S_size) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_size
//       Access: Published
//  Description: Removes the size specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_size() {
  _specified &= ~S_size;
  _x_size = 0;
  _y_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_title
//       Access: Published
//  Description: Specifies the title that should be assigned to the
//               window.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_title(const string &title) {
  _title = title;
  _specified |= S_title;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_title
//       Access: Published
//  Description: Returns the window's title.
////////////////////////////////////////////////////////////////////
INLINE const string &WindowProperties::
get_title() const {
  nassertr(has_title(), _title);
  return _title;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_title
//       Access: Published
//  Description: Returns true if the window title has been specified,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_title() const {
  return ((_specified & S_title) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_title
//       Access: Published
//  Description: Removes the title specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_title() {
  _specified &= ~S_title;
  _title = string();
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_undecorated
//       Access: Published
//  Description: Specifies whether the window should be created with a
//               visible title and border (false, the default) or not
//               (true).
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_undecorated(bool undecorated) {
  if (undecorated) {
    _flags |= F_undecorated;
  } else {
    _flags &= ~F_undecorated;
  }
  _specified |= S_undecorated;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_undecorated
//       Access: Published
//  Description: Returns true if the window has no border.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
get_undecorated() const {
  return (_flags & F_undecorated) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_undecorated
//       Access: Published
//  Description: Returns true if set_undecorated() has been specified.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_undecorated() const {
  return ((_specified & S_undecorated) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_undecorated
//       Access: Published
//  Description: Removes the undecorated specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_undecorated() {
  _specified &= ~S_undecorated;
  _flags &= ~F_undecorated;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_fullscreen
//       Access: Published
//  Description: Specifies whether the window should be opened in
//               fullscreen mode (true) or normal windowed mode
//               (false, the default).
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_fullscreen(bool fullscreen) {
  if (fullscreen) {
    _flags |= F_fullscreen;
  } else {
    _flags &= ~F_fullscreen;
  }
  _specified |= S_fullscreen;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_fullscreen
//       Access: Published
//  Description: Returns true if the window is in fullscreen mode.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
get_fullscreen() const {
  return (_flags & F_fullscreen) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_fullscreen
//       Access: Published
//  Description: Returns true if set_fullscreen() has been specified.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_fullscreen() const {
  return ((_specified & S_fullscreen) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_fullscreen
//       Access: Published
//  Description: Removes the fullscreen specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_fullscreen() {
  _specified &= ~S_fullscreen;
  _flags &= ~F_fullscreen;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_foreground
//       Access: Published
//  Description: Specifies whether the window should be opened in
//               the foreground (true), or left in the background
//               (false).
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_foreground(bool foreground) {
  if (foreground) {
    _flags |= F_foreground;
  } else {
    _flags &= ~F_foreground;
  }
  _specified |= S_foreground;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_foreground
//       Access: Published
//  Description: Returns true if the window is in the foreground.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
get_foreground() const {
  return (_flags & F_foreground) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_foreground
//       Access: Published
//  Description: Returns true if set_foreground() has been specified.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_foreground() const {
  return ((_specified & S_foreground) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_foreground
//       Access: Published
//  Description: Removes the foreground specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_foreground() {
  _specified &= ~S_foreground;
  _flags &= ~F_foreground;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_minimized
//       Access: Published
//  Description: Specifies whether the window should be created
//               minimized (true), or normal (false).
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_minimized(bool minimized) {
  if (minimized) {
    _flags |= F_minimized;
  } else {
    _flags &= ~F_minimized;
  }
  _specified |= S_minimized;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_minimized
//       Access: Published
//  Description: Returns true if the window is minimized.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
get_minimized() const {
  return (_flags & F_minimized) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_minimized
//       Access: Published
//  Description: Returns true if set_minimized() has been specified.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_minimized() const {
  return ((_specified & S_minimized) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_minimized
//       Access: Published
//  Description: Removes the minimized specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_minimized() {
  _specified &= ~S_minimized;
  _flags &= ~F_minimized;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_open
//       Access: Published
//  Description: Specifies whether the window should be open.  It is
//               legal to create a GraphicsWindow in the closed state,
//               and later request it to open by changing this flag.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_open(bool open) {
  if (open) {
    _flags |= F_open;
  } else {
    _flags &= ~F_open;
  }
  _specified |= S_open;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_open
//       Access: Published
//  Description: Returns true if the window is open.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
get_open() const {
  return (_flags & F_open) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_open
//       Access: Published
//  Description: Returns true if set_open() has been specified.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_open() const {
  return ((_specified & S_open) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_open
//       Access: Published
//  Description: Removes the open specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_open() {
  _specified &= ~S_open;
  _flags &= ~F_open;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_cursor_hidden
//       Access: Published
//  Description: Specifies whether the mouse cursor should be visible.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
set_cursor_hidden(bool cursor_hidden) {
  if (cursor_hidden) {
    _flags |= F_cursor_hidden;
  } else {
    _flags &= ~F_cursor_hidden;
  }
  _specified |= S_cursor_hidden;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::set_cursor_hidden
//       Access: Published
//  Description: Returns true if the mouse cursor is invisible.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
get_cursor_hidden() const {
  return (_flags & F_cursor_hidden) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::has_cursor_hidden
//       Access: Published
//  Description: Returns true if set_cursor_hidden() has been specified.
////////////////////////////////////////////////////////////////////
INLINE bool WindowProperties::
has_cursor_hidden() const {
  return ((_specified & S_cursor_hidden) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear_cursor_hidden
//       Access: Published
//  Description: Removes the cursor_hidden specification from the properties.
////////////////////////////////////////////////////////////////////
INLINE void WindowProperties::
clear_cursor_hidden() {
  _specified &= ~S_cursor_hidden;
  _flags &= ~F_cursor_hidden;
}

INLINE ostream &
operator << (ostream &out, const WindowProperties &properties) {
  properties.output(out);
  return out;
}
