// Filename: colorAttrib.I
// Created by:  drose (22Feb02)
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


////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::Constructor
//       Access: Private
//  Description: Use ColorAttrib::make() to construct a new
//               ColorAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorAttrib::
ColorAttrib(ColorAttrib::Type type, const LColor &color) :
  _type(type),
  _color(color)
{
  quantize_color();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::get_color_type
//       Access: Published
//  Description: Returns the type of color specified by this
//               ColorAttrib.  The options are:
//
//                 T_vertex - use the vertex color specified in the
//                 geometry itself.
//
//                 T_flat - use the color specified in this
//                 ColorAttrib for all geometry.  You can get this
//                 color via get_color().
//
//                 T_off - use the color white.
////////////////////////////////////////////////////////////////////
INLINE ColorAttrib::Type ColorAttrib::
get_color_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::get_color
//       Access: Published
//  Description: If the type is T_flat or T_off, this returns the
//               color that will be applied to geometry.  If the type
//               is T_vertex, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE const LColor &ColorAttrib::
get_color() const {
  return _color;
}
