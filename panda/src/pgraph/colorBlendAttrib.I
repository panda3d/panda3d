// Filename: colorBlendAttrib.I
// Created by:  drose (29Mar02)
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
//     Function: ColorBlendAttrib::Constructor
//       Access: Private
//  Description: Use ColorBlendAttrib::make() to construct a new
//               ColorBlendAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorBlendAttrib::
ColorBlendAttrib() :
  _mode(M_none),
  _a(O_one),
  _b(O_one),
  _color(Colorf::zero())
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::Constructor
//       Access: Private
//  Description: Use ColorBlendAttrib::make() to construct a new
//               ColorBlendAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorBlendAttrib::
ColorBlendAttrib(ColorBlendAttrib::Mode mode,
                 ColorBlendAttrib::Operand a, ColorBlendAttrib::Operand b,
                 const Colorf &color) :
  _mode(mode),
  _a(a),
  _b(b),
  _color(color)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::get_mode
//       Access: Published
//  Description: Returns the colorBlend mode.
////////////////////////////////////////////////////////////////////
INLINE ColorBlendAttrib::Mode ColorBlendAttrib::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::get_operand_a
//       Access: Published
//  Description: Returns the multiplier for the first component.
////////////////////////////////////////////////////////////////////
INLINE ColorBlendAttrib::Operand ColorBlendAttrib::
get_operand_a() const {
  return _a;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::get_operand_b
//       Access: Published
//  Description: Returns the multiplier for the second component.
////////////////////////////////////////////////////////////////////
INLINE ColorBlendAttrib::Operand ColorBlendAttrib::
get_operand_b() const {
  return _b;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::get_color
//       Access: Published
//  Description: Returns the constant color associated with the attrib.
////////////////////////////////////////////////////////////////////
INLINE Colorf ColorBlendAttrib::
get_color() const {
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::involves_constant_color
//       Access: Published, Static
//  Description: Returns true if the indicated operand uses the
//               constant color, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ColorBlendAttrib::
involves_constant_color(Operand operand) {
  switch (operand) {
  case O_constant_color:
  case O_one_minus_constant_color:
  case O_constant_alpha:
  case O_one_minus_constant_alpha:
    return true;

  default:
    return false;
  }
}
