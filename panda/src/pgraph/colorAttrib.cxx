// Filename: colorAttrib.cxx
// Created by:  drose (22Feb02)
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

#include "colorAttrib.h"
#include "dcast.h"

TypeHandle ColorAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::make_vertex
//       Access: Published, Static
//  Description: Constructs a new ColorAttrib object that indicates
//               geometry should be rendered according to its own
//               vertex color.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorAttrib::
make_vertex() {
  ColorAttrib *attrib = new ColorAttrib(T_vertex, Colorf(0.0f, 0.0f, 0.0f, 1.0f));
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::make_flat
//       Access: Published, Static
//  Description: Constructs a new ColorAttrib object that indicates
//               geometry should be rendered in the indicated color.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorAttrib::
make_flat(const Colorf &color) {
  ColorAttrib *attrib = new ColorAttrib(T_flat, color);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new ColorAttrib object that indicates
//               geometry should be rendered without any color
//               commands at all.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorAttrib::
make_off() {
  ColorAttrib *attrib = new ColorAttrib(T_off, Colorf(0.0f, 0.0f, 0.0f, 1.0f));
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ColorAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  switch (get_color_type()) {
  case T_vertex:
    out << "vertex";
    break;

  case T_flat:
    out << "(" << get_color() << ")";
    break;

  case T_off:
    out << "off";
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ColorAttrib
//               types to return a unique number indicating whether
//               this ColorAttrib is equivalent to the other one.
//
//               This should return 0 if the two ColorAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two ColorAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int ColorAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ColorAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  if (_type != ta->_type) {
    return (int)_type - (int)ta->_type;
  }
  if (_type == T_flat) {
    return _color.compare_to(ta->_color);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ColorAttrib
//               types to specify what the default property for a
//               ColorAttrib of this type should be.
//
//               This should return a newly-allocated ColorAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of ColorAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *ColorAttrib::
make_default_impl() const {
  return new ColorAttrib(T_vertex, Colorf(0.0f, 0.0f, 0.0f, 1.0f));
}
