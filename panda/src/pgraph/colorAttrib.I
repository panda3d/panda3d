// Filename: colorAttrib.I
// Created by:  drose (22Feb02)
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
//     Function: ColorAttrib::Constructor
//       Access: Private
//  Description: Use ColorAttrib::make() to construct a new
//               ColorAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorAttrib::
ColorAttrib(ColorAttrib::Type type, const Colorf &color) :
  _type(type),
  _color(color)
{
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
//                 T_off - do not issue any color commands at all.
//                 This is generally used only in contexts where the
//                 color is meaningless, e.g. when drawing directly to
//                 the depth buffer.
////////////////////////////////////////////////////////////////////
INLINE ColorAttrib::Type ColorAttrib::
get_color_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttrib::get_color
//       Access: Published
//  Description: If the type is T_flat, this returns the color that
//               will be applied to geometry.  If the type is anything
//               else, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE const Colorf &ColorAttrib::
get_color() const {
  return _color;
}
