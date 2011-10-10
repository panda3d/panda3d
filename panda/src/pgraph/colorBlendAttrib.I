// Filename: colorBlendAttrib.I
// Created by:  drose (29Mar02)
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
  _color(LColor::zero()),
  _involves_constant_color(false),
  _involves_color_scale(false)
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
                 const LColor &color) :
  _mode(mode),
  _a(a),
  _b(b),
  _color(color),
  _involves_constant_color(involves_constant_color(a) || involves_constant_color(b)),
  _involves_color_scale(involves_color_scale(a) || involves_color_scale(b))
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
INLINE LColor ColorBlendAttrib::
get_color() const {
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::involves_constant_color
//       Access: Published
//  Description: Returns true if the this attrib uses the
//               constant color, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ColorBlendAttrib::
involves_constant_color() const {
  return _involves_constant_color;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::involves_color_scale
//       Access: Published
//  Description: Returns true if the this attrib uses the
//               color scale attrib, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ColorBlendAttrib::
involves_color_scale() const {
  return _involves_color_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::involves_constant_color
//       Access: Published, Static
//  Description: Returns true if the indicated operand uses the
//               constant color, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ColorBlendAttrib::
involves_constant_color(ColorBlendAttrib::Operand operand) {
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

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::involves_color_scale
//       Access: Published, Static
//  Description: Returns true if the indicated operand uses the
//               color scale attrib, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ColorBlendAttrib::
involves_color_scale(ColorBlendAttrib::Operand operand) {
  switch (operand) {
  case O_color_scale:
  case O_one_minus_color_scale:
  case O_alpha_scale:
  case O_one_minus_alpha_scale:
    return true;

  default:
    return false;
  }
}
