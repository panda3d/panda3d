// Filename: windowProperties.h
// Created by:  drose (13Aug02)
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

#ifndef WINDOWPROPERTIES_H
#define WINDOWPROPERTIES_H

#include "pandabase.h"
#include "filename.h"
#include "pnotify.h"
#include "windowHandle.h"
#include "lpoint2.h"
#include "lvector2.h"

////////////////////////////////////////////////////////////////////
//       Class : WindowProperties
// Description : A container for the various kinds of properties we
//               might ask to have on a graphics window before we open
//               it.  This also serves to hold the current properties
//               for a window after it has been opened.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY WindowProperties {
PUBLISHED:
  enum ZOrder {
    Z_bottom,
    Z_normal,
    Z_top,
  };
  
  enum MouseMode {
    M_absolute,
    M_relative,
    M_confined,
  };

  WindowProperties();
  INLINE WindowProperties(const WindowProperties &copy);
  void operator = (const WindowProperties &copy);
  INLINE ~WindowProperties();

  static WindowProperties get_config_properties();
  static WindowProperties get_default();
  static void set_default(const WindowProperties &default_properties);
  static void clear_default();

  static WindowProperties size(int x_size, int y_size);

  bool operator == (const WindowProperties &other) const;
  INLINE bool operator != (const WindowProperties &other) const;

  void clear();
  INLINE bool is_any_specified() const;

  INLINE void set_origin(const LPoint2i &origin);
  INLINE void set_origin(int x_origin, int y_origin);
  INLINE const LPoint2i &get_origin() const;
  INLINE int get_x_origin() const;
  INLINE int get_y_origin() const;
  INLINE bool has_origin() const;
  INLINE void clear_origin();

  INLINE void set_size(const LVector2i &size);
  INLINE void set_size(int x_size, int y_size);
  INLINE const LVector2i &get_size() const;
  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE bool has_size() const;
  INLINE void clear_size();

  INLINE bool has_mouse_mode() const;
  INLINE void set_mouse_mode(MouseMode mode);
  INLINE MouseMode get_mouse_mode() const;
  INLINE void clear_mouse_mode();

  INLINE void set_title(const string &title);
  INLINE const string &get_title() const;
  INLINE bool has_title() const;
  INLINE void clear_title();

  INLINE void set_undecorated(bool undecorated);
  INLINE bool get_undecorated() const;
  INLINE bool has_undecorated() const;
  INLINE void clear_undecorated();

  INLINE void set_fixed_size(bool fixed_size);
  INLINE bool get_fixed_size() const;
  INLINE bool has_fixed_size() const;
  INLINE void clear_fixed_size();

  INLINE void set_fullscreen(bool fullscreen);
  INLINE bool get_fullscreen() const;
  INLINE bool has_fullscreen() const;
  INLINE void clear_fullscreen();

  INLINE void set_foreground(bool foreground);
  INLINE bool get_foreground() const;
  INLINE bool has_foreground() const;
  INLINE void clear_foreground();

  INLINE void set_minimized(bool minimized);
  INLINE bool get_minimized() const;
  INLINE bool has_minimized() const;
  INLINE void clear_minimized();

  INLINE void set_raw_mice(bool raw_mice);
  INLINE bool get_raw_mice() const;
  INLINE bool has_raw_mice() const;
  INLINE void clear_raw_mice();

  INLINE void set_open(bool open);
  INLINE bool get_open() const;
  INLINE bool has_open() const;
  INLINE void clear_open();

  INLINE void set_cursor_hidden(bool cursor_hidden);
  INLINE bool get_cursor_hidden() const;
  INLINE bool has_cursor_hidden() const;
  INLINE void clear_cursor_hidden();

  INLINE void set_icon_filename(const Filename &icon_filename);
  INLINE const Filename &get_icon_filename() const;
  INLINE bool has_icon_filename() const;
  INLINE void clear_icon_filename();

  INLINE void set_cursor_filename(const Filename &cursor_filename);
  INLINE const Filename &get_cursor_filename() const;
  INLINE bool has_cursor_filename() const;
  INLINE void clear_cursor_filename();

  INLINE void set_z_order(ZOrder z_order);
  INLINE ZOrder get_z_order() const;
  INLINE bool has_z_order() const;
  INLINE void clear_z_order();

  void set_parent_window(size_t parent);
  INLINE void set_parent_window(WindowHandle *parent_window = NULL);
  INLINE WindowHandle *get_parent_window() const;
  INLINE bool has_parent_window() const;
  INLINE void clear_parent_window();

  void add_properties(const WindowProperties &other);

  void output(ostream &out) const;

private:
  // This bitmask indicates which of the parameters in the properties
  // structure have been filled in by the user, and which remain
  // unspecified.
  enum Specified {
    S_origin               = 0x00001,
    S_size                 = 0x00002,
    S_title                = 0x00004,
    S_undecorated          = 0x00008,
    S_fullscreen           = 0x00010,
    S_foreground           = 0x00020,
    S_minimized            = 0x00040,
    S_open                 = 0x00080,
    S_cursor_hidden        = 0x00100,
    S_fixed_size           = 0x00200,
    S_z_order              = 0x00400,
    S_icon_filename        = 0x00800,
    S_cursor_filename      = 0x01000,
    S_mouse_mode           = 0x02000,
    S_parent_window        = 0x04000,
    S_raw_mice             = 0x08000,
  };

  // This bitmask represents the true/false settings for various
  // boolean flags (assuming the corresponding S_* bit has been set,
  // above).
  enum Flags {
    F_undecorated    = S_undecorated,
    F_fullscreen     = S_fullscreen,
    F_foreground     = S_foreground,
    F_minimized      = S_minimized,
    F_open           = S_open,
    F_cursor_hidden  = S_cursor_hidden,
    F_fixed_size     = S_fixed_size,
    F_raw_mice       = S_raw_mice,
  };

  int _specified;
  LPoint2i _origin;
  LVector2i _size;
  MouseMode _mouse_mode;
  string _title;
  Filename _cursor_filename;
  Filename _icon_filename;
  ZOrder _z_order;
  unsigned int _flags;
  PT(WindowHandle) _parent_window;

  static WindowProperties *_default_properties;
};

EXPCL_PANDA_DISPLAY ostream &
operator << (ostream &out, WindowProperties::ZOrder z_order);
EXPCL_PANDA_DISPLAY istream &
operator >> (istream &in, WindowProperties::ZOrder &z_order);

EXPCL_PANDA_DISPLAY ostream &
operator << (ostream &out, WindowProperties::MouseMode mode);
EXPCL_PANDA_DISPLAY istream &
operator >> (istream &in, WindowProperties::MouseMode &mode);


INLINE ostream &operator << (ostream &out, const WindowProperties &properties);

#include "windowProperties.I"

#endif
