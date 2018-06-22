/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowProperties.I
 * @author drose
 * @date 2002-08-13
 */

/**
 *
 */
INLINE WindowProperties::
WindowProperties(const WindowProperties &copy) {
  (*this) = copy;
}

/**
 *
 */
INLINE WindowProperties::
~WindowProperties() {
}

/**
 *
 */
INLINE bool WindowProperties::
operator != (const WindowProperties &other) const {
  return !operator == (other);
}

/**
 * Returns true if any properties have been specified, false otherwise.
 */
INLINE bool WindowProperties::
is_any_specified() const {
  return (_specified != 0);
}

/**
 * Specifies the origin on the screen (in pixels, relative to the top-left
 * corner) at which the window should appear.  This is the origin of the top-
 * left corner of the useful part of the window, not including decorations.
 */
INLINE void WindowProperties::
set_origin(const LPoint2i &origin) {
  _origin = origin;
  _specified |= S_origin;
}

/**
 * Specifies the origin on the screen (in pixels, relative to the top-left
 * corner) at which the window should appear.  This is the origin of the top-
 * left corner of the useful part of the window, not including decorations.
 */
INLINE void WindowProperties::
set_origin(int x_origin, int y_origin) {
  _origin.set(x_origin, y_origin);
  _specified |= S_origin;
}

/**
 * Returns the coordinates of the window's top-left corner, not including
 * decorations.
 */
INLINE const LPoint2i &WindowProperties::
get_origin() const {
  nassertr(has_origin(), LPoint2i::zero());
  return _origin;
}

/**
 * Returns the x coordinate of the window's top-left corner, not including
 * decorations.
 */
INLINE int WindowProperties::
get_x_origin() const {
  nassertr(has_origin(), 0);
  return _origin.get_x();
}

/**
 * Returns the y coordinate of the window's top-left corner, not including
 * decorations.
 */
INLINE int WindowProperties::
get_y_origin() const {
  nassertr(has_origin(), 0);
  return _origin.get_y();
}

/**
 * Returns true if the window origin has been specified, false otherwise.
 */
INLINE bool WindowProperties::
has_origin() const {
  return ((_specified & S_origin) != 0);
}

/**
 * Removes the origin specification from the properties.
 */
INLINE void WindowProperties::
clear_origin() {
  _specified &= ~S_origin;
  _origin = LPoint2i::zero();
}

/**
 * Specifies the requested size of the window, in pixels.  This is the size of
 * the useful part of the window, not including decorations.
 */
INLINE void WindowProperties::
set_size(const LVector2i &size) {
  _size = size;
  _specified |= S_size;
}

/**
 * Specifies the requested size of the window, in pixels.  This is the size of
 * the useful part of the window, not including decorations.
 */
INLINE void WindowProperties::
set_size(int x_size, int y_size) {
  _size.set(x_size, y_size);
  _specified |= S_size;
}

/**
 * Returns size in pixels of the useful part of the window, not including
 * decorations.
 */
INLINE const LVector2i &WindowProperties::
get_size() const {
  nassertr(has_size(), LVector2i::zero());
  return _size;
}

/**
 * Returns size in pixels in the x dimension of the useful part of the window,
 * not including decorations.  That is, this is the window's width.
 */
INLINE int WindowProperties::
get_x_size() const {
  nassertr(has_size(), 0);
  return _size.get_x();
}

/**
 * Returns size in pixels in the y dimension of the useful part of the window,
 * not including decorations.  That is, this is the window's height.
 */
INLINE int WindowProperties::
get_y_size() const {
  nassertr(has_size(), 0);
  return _size.get_y();
}

/**
 * Returns true if the window size has been specified, false otherwise.
 */
INLINE bool WindowProperties::
has_size() const {
  return ((_specified & S_size) != 0);
}

/**
 * Removes the size specification from the properties.
 */
INLINE void WindowProperties::
clear_size() {
  _specified &= ~S_size;
  _size = LVector2i::zero();
}

/**
 * Specifies the title that should be assigned to the window.
 */
INLINE void WindowProperties::
set_title(const std::string &title) {
  _title = title;
  _specified |= S_title;
}

/**
 * Returns the window's title.
 */
INLINE const std::string &WindowProperties::
get_title() const {
  nassertr(has_title(), _title);
  return _title;
}

/**
 * Returns true if the window title has been specified, false otherwise.
 */
INLINE bool WindowProperties::
has_title() const {
  return ((_specified & S_title) != 0);
}

/**
 * Removes the title specification from the properties.
 */
INLINE void WindowProperties::
clear_title() {
  _specified &= ~S_title;
  _title = std::string();
}

/**
 * Specifies whether the window should be created with a visible title and
 * border (false, the default) or not (true).
 */
INLINE void WindowProperties::
set_undecorated(bool undecorated) {
  if (undecorated) {
    _flags |= F_undecorated;
  } else {
    _flags &= ~F_undecorated;
  }
  _specified |= S_undecorated;
}

/**
 * Returns true if the window has no border.
 */
INLINE bool WindowProperties::
get_undecorated() const {
  return (_flags & F_undecorated) != 0;
}

/**
 * Returns true if set_undecorated() has been specified.
 */
INLINE bool WindowProperties::
has_undecorated() const {
  return ((_specified & S_undecorated) != 0);
}

/**
 * Removes the undecorated specification from the properties.
 */
INLINE void WindowProperties::
clear_undecorated() {
  _specified &= ~S_undecorated;
  _flags &= ~F_undecorated;
}

/**
 * Specifies whether the window should be resizable by the user.
 */
INLINE void WindowProperties::
set_fixed_size(bool fixed_size) {
  if (fixed_size) {
    _flags |= F_fixed_size;
  } else {
    _flags &= ~F_fixed_size;
  }
  _specified |= S_fixed_size;
}

/**
 * Returns true if the window cannot be resized by the user, false otherwise.
 */
INLINE bool WindowProperties::
get_fixed_size() const {
  return (_flags & F_fixed_size) != 0;
}

/**
 * Returns true if set_fixed_size() has been specified.
 */
INLINE bool WindowProperties::
has_fixed_size() const {
  return ((_specified & S_fixed_size) != 0);
}

/**
 * Removes the fixed_size specification from the properties.
 */
INLINE void WindowProperties::
clear_fixed_size() {
  _specified &= ~S_fixed_size;
  _flags &= ~F_fixed_size;
}

/**
 * Specifies whether the window should be opened in fullscreen mode (true) or
 * normal windowed mode (false, the default).
 */
INLINE void WindowProperties::
set_fullscreen(bool fullscreen) {
  if (fullscreen) {
    _flags |= F_fullscreen;
  } else {
    _flags &= ~F_fullscreen;
  }
  _specified |= S_fullscreen;
}

/**
 * Returns true if the window is in fullscreen mode.
 */
INLINE bool WindowProperties::
get_fullscreen() const {
  return (_flags & F_fullscreen) != 0;
}

/**
 * Returns true if set_fullscreen() has been specified.
 */
INLINE bool WindowProperties::
has_fullscreen() const {
  return ((_specified & S_fullscreen) != 0);
}

/**
 * Removes the fullscreen specification from the properties.
 */
INLINE void WindowProperties::
clear_fullscreen() {
  _specified &= ~S_fullscreen;
  _flags &= ~F_fullscreen;
}

/**
 * Specifies whether the window should be opened in the foreground (true), or
 * left in the background (false).
 */
INLINE void WindowProperties::
set_foreground(bool foreground) {
  if (foreground) {
    _flags |= F_foreground;
  } else {
    _flags &= ~F_foreground;
  }
  _specified |= S_foreground;
}

/**
 * Returns true if the window is in the foreground.
 */
INLINE bool WindowProperties::
get_foreground() const {
  return (_flags & F_foreground) != 0;
}

/**
 * Returns true if set_foreground() has been specified.
 */
INLINE bool WindowProperties::
has_foreground() const {
  return ((_specified & S_foreground) != 0);
}

/**
 * Removes the foreground specification from the properties.
 */
INLINE void WindowProperties::
clear_foreground() {
  _specified &= ~S_foreground;
  _flags &= ~F_foreground;
}

/**
 * Specifies whether the window should be created minimized (true), or normal
 * (false).
 */
INLINE void WindowProperties::
set_minimized(bool minimized) {
  if (minimized) {
    _flags |= F_minimized;
  } else {
    _flags &= ~F_minimized;
  }
  _specified |= S_minimized;
}

/**
 * Returns true if the window is minimized.
 */
INLINE bool WindowProperties::
get_minimized() const {
  return (_flags & F_minimized) != 0;
}

/**
 * Returns true if set_minimized() has been specified.
 */
INLINE bool WindowProperties::
has_minimized() const {
  return ((_specified & S_minimized) != 0);
}

/**
 * Removes the minimized specification from the properties.
 */
INLINE void WindowProperties::
clear_minimized() {
  _specified &= ~S_minimized;
  _flags &= ~F_minimized;
}

/**
 * Specifies whether the window should read the raw mouse devices.
 */
INLINE void WindowProperties::
set_raw_mice(bool raw_mice) {
  if (raw_mice) {
    _flags |= F_raw_mice;
  } else {
    _flags &= ~F_raw_mice;
  }
  _specified |= S_raw_mice;
}

/**
 * Returns true if the window reads the raw mice.
 */
INLINE bool WindowProperties::
get_raw_mice() const {
  return (_flags & F_raw_mice) != 0;
}

/**
 * Returns true if set_raw_mice() has been specified.
 */
INLINE bool WindowProperties::
has_raw_mice() const {
  return ((_specified & S_raw_mice) != 0);
}

/**
 * Removes the raw_mice specification from the properties.
 */
INLINE void WindowProperties::
clear_raw_mice() {
  _specified &= ~S_raw_mice;
  _flags &= ~F_raw_mice;
}

/**
 * Specifies whether the window should be open.  It is legal to create a
 * GraphicsWindow in the closed state, and later request it to open by
 * changing this flag.
 */
INLINE void WindowProperties::
set_open(bool open) {
  if (open) {
    _flags |= F_open;
  } else {
    _flags &= ~F_open;
  }
  _specified |= S_open;
}

/**
 * Returns true if the window is open.
 */
INLINE bool WindowProperties::
get_open() const {
  return (_flags & F_open) != 0;
}

/**
 * Returns true if set_open() has been specified.
 */
INLINE bool WindowProperties::
has_open() const {
  return ((_specified & S_open) != 0);
}

/**
 * Removes the open specification from the properties.
 */
INLINE void WindowProperties::
clear_open() {
  _specified &= ~S_open;
  _flags &= ~F_open;
}

/**
 * Specifies whether the mouse cursor should be visible.
 */
INLINE void WindowProperties::
set_cursor_hidden(bool cursor_hidden) {
  if (cursor_hidden) {
    _flags |= F_cursor_hidden;
  } else {
    _flags &= ~F_cursor_hidden;
  }
  _specified |= S_cursor_hidden;
}

/**
 * Returns true if the mouse cursor is invisible.
 */
INLINE bool WindowProperties::
get_cursor_hidden() const {
  return (_flags & F_cursor_hidden) != 0;
}

/**
 * Returns true if set_cursor_hidden() has been specified.
 */
INLINE bool WindowProperties::
has_cursor_hidden() const {
  return ((_specified & S_cursor_hidden) != 0);
}

/**
 * Removes the cursor_hidden specification from the properties.
 */
INLINE void WindowProperties::
clear_cursor_hidden() {
  _specified &= ~S_cursor_hidden;
  _flags &= ~F_cursor_hidden;
}

/**
 * Specifies the file that contains the icon to associate with the window when
 * it is minimized.
 */
INLINE void WindowProperties::
set_icon_filename(const Filename &icon_filename) {
  _icon_filename = icon_filename;
  _specified |= S_icon_filename;
}


/**
 * Returns the icon filename associated with the window.
 */
INLINE const Filename &WindowProperties::
get_icon_filename() const {
  return _icon_filename;
}

/**
 * Returns true if set_icon_filename() has been specified.
 */
INLINE bool WindowProperties::
has_icon_filename() const {
  return ((_specified & S_icon_filename) != 0);
}

/**
 * Removes the icon_filename specification from the properties.
 */
INLINE void WindowProperties::
clear_icon_filename() {
  _specified &= ~S_icon_filename;
  _icon_filename = Filename();
}

/**
 * Specifies the file that contains the icon to associate with the mouse
 * cursor when it is within the window (and visible).
 */
INLINE void WindowProperties::
set_cursor_filename(const Filename &cursor_filename) {
  _cursor_filename = cursor_filename;
  _specified |= S_cursor_filename;
}

/**
 * Returns the icon filename associated with the mouse cursor.
 */
INLINE const Filename &WindowProperties::
get_cursor_filename() const {
  return _cursor_filename;
}

/**
 * Returns true if set_cursor_filename() has been specified.
 */
INLINE bool WindowProperties::
has_cursor_filename() const {
  return ((_specified & S_cursor_filename) != 0);
}

/**
 * Removes the cursor_filename specification from the properties.
 */
INLINE void WindowProperties::
clear_cursor_filename() {
  _specified &= ~S_cursor_filename;
  _cursor_filename = Filename();
}

/**
 * Specifies the relative ordering of the window with respect to other
 * windows.  If the z_order is Z_top, the window will always be on top of
 * other windows; if it is Z_bottom, it will always be below other windows.
 * Most windows will want to be Z_normal, which allows the user to control the
 * order.
 */
INLINE void WindowProperties::
set_z_order(WindowProperties::ZOrder z_order) {
  _z_order = z_order;
  _specified |= S_z_order;
}

/**
 * Returns the window's z_order.
 */
INLINE WindowProperties::ZOrder WindowProperties::
get_z_order() const {
  return _z_order;
}

/**
 * Returns true if the window z_order has been specified, false otherwise.
 */
INLINE bool WindowProperties::
has_z_order() const {
  return ((_specified & S_z_order) != 0);
}

/**
 * Removes the z_order specification from the properties.
 */
INLINE void WindowProperties::
clear_z_order() {
  _specified &= ~S_z_order;
  _z_order = Z_normal;
}


/**
 * Specifies the mode in which the window is to operate its mouse pointer.
 *
 * M_absolute: the normal mode in which a mouse pointer operates, where the
 * mouse can move outside the window and the mouse coordinates are relative to
 * its position in the window.
 *
 * M_relative (OSX or Unix/X11 only): a mode where only relative movements are
 * reported; particularly useful for FPS-style mouse movements where you have
 * hidden the mouse pointer and are are more interested in how fast the mouse
 * is moving, rather than precisely where the pointer is hovering.
 *
 * This has no effect on Windows.  On Unix/X11, this requires the Xxf86dga
 * extension to be available.
 *
 * M_confined: this mode reports absolute mouse positions, but confines the
 * mouse pointer to the window boundary.  It can portably replace M_relative
 * for an FPS, but you need to periodically move the pointer to the center of
 * the window and track movement deltas.
 *
 */
INLINE void WindowProperties::
set_mouse_mode(MouseMode mode) {
  _mouse_mode=mode;
  _specified |= S_mouse_mode;
}

/**
 * See set_mouse_mode().
 */
INLINE WindowProperties::MouseMode WindowProperties::
get_mouse_mode() const {
 return _mouse_mode;
}

/**
 *
 */
INLINE bool WindowProperties::
has_mouse_mode() const {
  return ((_specified & S_mouse_mode)!=0);
}

/**
 * Removes the mouse_mode specification from the properties.
 */
INLINE void WindowProperties::
clear_mouse_mode() {
  _specified &= ~S_mouse_mode;
  _mouse_mode = M_absolute;
}

/**
 * Specifies the window that this window should be attached to.  If this is
 * NULL or unspecified, the window will be created as a toplevel window on the
 * desktop; if this is non-NULL, the window will be bound as a child window to
 * the indicated parent window.
 *
 * You should use GraphicsPipe::make_window_handle() to create an instance of
 * a WindowHandle object given an appropriate OS-specific window handle
 * representation.  Each OS-specific GraphicsPipe class defines a
 * make_window_handle() method that returns an appropriate WindowHandle object
 * to wrap the particular OS-specific representation.
 */
INLINE void WindowProperties::
set_parent_window(WindowHandle *parent_window) {
  _parent_window = parent_window;
  _specified |= S_parent_window;
}

/**
 * Returns the parent window specification, or NULL if there is no parent
 * window specified.
 */
INLINE WindowHandle *WindowProperties::
get_parent_window() const {
 return _parent_window;
}

/**
 * Checks the S_parent_window specification from the properties.
 */
INLINE bool WindowProperties::
has_parent_window() const {
  return ((_specified & S_parent_window)!=0);
}

/**
 * Removes the S_parent_window specification from the properties.
 */
INLINE void WindowProperties::
clear_parent_window() {
  _specified &= ~S_parent_window;
  _parent_window = nullptr;
}


INLINE std::ostream &
operator << (std::ostream &out, const WindowProperties &properties) {
  properties.output(out);
  return out;
}
