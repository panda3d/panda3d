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
  _flags = copy._flags;
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
          _title == other._title);
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
}
