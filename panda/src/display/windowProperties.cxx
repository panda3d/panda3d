// Filename: windowProperties.cxx
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

#include "windowProperties.h"


////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
WindowProperties::
WindowProperties() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void WindowProperties::
operator = (const WindowProperties &copy) {
  _specified = copy._specified;
  _x_origin = copy._x_origin;
  _y_origin = copy._y_origin;
  _x_size = copy._x_size;
  _y_size = copy._y_size;
  _title = copy._title;
  _icon_filename = copy._icon_filename;
  _cursor_filename = copy._cursor_filename;
  _z_order = copy._z_order;
  _flags = copy._flags;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::get_default
//       Access: Published, Static
//  Description: Returns a WindowProperties structure with all of the
//               default values filled in according to the user's
//               config file.
////////////////////////////////////////////////////////////////////
WindowProperties WindowProperties::
get_default() {
  WindowProperties props;

  props.set_open(true);

  if (win_width.has_value() && win_height.has_value() &&
      !win_size.has_value()) {
    props.set_size(win_width, win_height);

  } else {
    if (win_size.get_num_words() == 1) {
      props.set_size(win_size[0], win_size[0]);
    } else {
      props.set_size(win_size[0], win_size[1]);
    }
  }

  if (win_origin_x.has_value() && win_origin_y.has_value() && 
      !win_origin.has_value()) {
    props.set_origin(win_origin_x, win_origin_y);

  } else {
    props.set_origin(win_origin[0], win_origin[1]);
  }

  props.set_fullscreen(fullscreen);
  props.set_undecorated(undecorated);
  props.set_cursor_hidden(cursor_hidden);
  if (icon_filename.has_value()) {
    props.set_icon_filename(icon_filename);
  }
  if (cursor_filename.has_value()) {
    props.set_cursor_filename(cursor_filename);
  }
  if (z_order.has_value()) {
    props.set_z_order(z_order);
  }
  props.set_title(window_title);

  return props;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::operator == 
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool WindowProperties::
operator == (const WindowProperties &other) const {
  return (_specified == other._specified &&
          _flags == other._flags &&
          _x_origin == other._x_origin &&
          _y_origin == other._y_origin &&
          _x_size == other._x_size &&
          _y_size == other._y_size &&
          _z_order == other._z_order &&
          _title == other._title &&
          _icon_filename == other._icon_filename &&
          _cursor_filename == other._cursor_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::clear
//       Access: Published
//  Description: Unsets all properties that have been specified so
//               far, and resets the WindowProperties structure to its
//               initial empty state.
////////////////////////////////////////////////////////////////////
void WindowProperties::
clear() {
  _specified = 0;
  _x_origin = 0;
  _y_origin = 0;
  _x_size = 0;
  _y_size = 0;
  _title = string();
  _icon_filename = Filename();
  _cursor_filename = Filename();
  _z_order = Z_normal;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::add_properties
//       Access: Published
//  Description: Sets any properties that are explicitly specified in
//               other on this object.  Leaves other properties
//               unchanged.
////////////////////////////////////////////////////////////////////
void WindowProperties::
add_properties(const WindowProperties &other) {
  if (other.has_origin()) {
    set_origin(other.get_x_origin(), other.get_y_origin());
  }
  if (other.has_size()) {
    set_size(other.get_x_size(), other.get_y_size());
  }
  if (other.has_title()) {
    set_title(other.get_title());
  }
  if (other.has_undecorated()) {
    set_undecorated(other.get_undecorated());
  }
  if (other.has_fixed_size()) {
    set_fixed_size(other.get_fixed_size());
  }
  if (other.has_fullscreen()) {
    set_fullscreen(other.get_fullscreen());
  }
  if (other.has_foreground()) {
    set_foreground(other.get_foreground());
  }
  if (other.has_minimized()) {
    set_minimized(other.get_minimized());
  }
  if (other.has_open()) {
    set_open(other.get_open());
  }
  if (other.has_cursor_hidden()) {
    set_cursor_hidden(other.get_cursor_hidden());
  }
  if (other.has_icon_filename()) {
    set_icon_filename(other.get_icon_filename());
  }
  if (other.has_cursor_filename()) {
    set_cursor_filename(other.get_cursor_filename());
  }
  if (other.has_z_order()) {
    set_z_order(other.get_z_order());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WindowProperties::output
//       Access: Published
//  Description: Sets any properties that are explicitly specified in
//               other on this object.  Leaves other properties
//               unchanged.
////////////////////////////////////////////////////////////////////
void WindowProperties::
output(ostream &out) const {
  if (has_origin()) {
    out << "origin=(" << get_x_origin() << ", " << get_y_origin() << ") ";
  }
  if (has_size()) {
    out << "size=(" << get_x_size() << ", " << get_y_size() << ") ";
  }
  if (has_title()) {
    out << "title=\"" << get_title() << "\"" << " ";
  }
  if (has_undecorated()) {
    out << (get_undecorated() ? "undecorated " : "!undecorated ");
  }
  if (has_fixed_size()) {
    out << (get_fixed_size() ? "fixed_size " : "!fixed_size ");
  }
  if (has_fullscreen()) {
    out << (get_fullscreen() ? "fullscreen " : "!fullscreen ");
  }
  if (has_foreground()) {
    out << (get_foreground() ? "foreground " : "!foreground ");
  }
  if (has_minimized()) {
    out << (get_minimized() ? "minimized " : "!minimized ");
  }
  if (has_open()) {
    out << (get_open() ? "open " : "!open ");
  }
  if (has_cursor_hidden()) {
    out << (get_cursor_hidden() ? "cursor_hidden " : "!cursor_hidden ");
  }
  if (has_icon_filename()) {
    out << "icon:" << get_icon_filename() << " ";
  }
  if (has_cursor_filename()) {
    out << "cursor:" << get_cursor_filename() << " ";
  }
  if (has_z_order()) {
    out << get_z_order() << " ";
  }
}

ostream &
operator << (ostream &out, WindowProperties::ZOrder z_order) {
  switch (z_order) {
  case WindowProperties::Z_bottom:
    return out << "bottom";

  case WindowProperties::Z_normal:
    return out << "normal";

  case WindowProperties::Z_top:
    return out << "top";
  }

  return out << "**invalid WindowProperties::ZOrder(" << (int)z_order << ")**";
}

istream &
operator >> (istream &in, WindowProperties::ZOrder &z_order) {
  string word;
  in >> word;

  if (word == "bottom") {
    z_order = WindowProperties::Z_bottom;

  } else if (word == "top") {
    z_order = WindowProperties::Z_top;

  } else if (word == "normal") {
    z_order = WindowProperties::Z_normal;

  } else {
    display_cat.warning()
      << "Unknown z-order: " << word << "\n";
    z_order = WindowProperties::Z_normal;
  }

  return in;
}
