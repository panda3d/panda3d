/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorBlendAttrib.I
 * @author drose
 * @date 2002-03-29
 */

/**
 * Use ColorBlendAttrib::make() to construct a new ColorBlendAttrib object.
 */
INLINE ColorBlendAttrib::
ColorBlendAttrib() :
  _mode(M_none),
  _a(O_one),
  _b(O_one),
  _alpha_mode(M_none),
  _alpha_a(O_one),
  _alpha_b(O_one),
  _color(LColor::zero()),
  _involves_constant_color(false),
  _involves_color_scale(false)
{
}

/**
 * Use ColorBlendAttrib::make() to construct a new ColorBlendAttrib object.
 */
INLINE ColorBlendAttrib::
ColorBlendAttrib(ColorBlendAttrib::Mode mode,
                 ColorBlendAttrib::Operand a, ColorBlendAttrib::Operand b,
                 ColorBlendAttrib::Mode alpha_mode,
                 ColorBlendAttrib::Operand alpha_a, ColorBlendAttrib::Operand alpha_b,
                 const LColor &color) :
  _mode(mode),
  _a(a),
  _b(b),
  _alpha_mode(alpha_mode),
  _alpha_a(alpha_a),
  _alpha_b(alpha_b),
  _color(color),
  _involves_constant_color(involves_constant_color(a) ||
                           involves_constant_color(b) ||
                           involves_constant_color(alpha_a) ||
                           involves_constant_color(alpha_b)),
  _involves_color_scale(involves_color_scale(a) ||
                        involves_color_scale(b) ||
                        involves_color_scale(alpha_a) ||
                        involves_color_scale(alpha_b))
{
}

/**
 * Returns the blending mode for the RGB channels.
 */
INLINE ColorBlendAttrib::Mode ColorBlendAttrib::
get_mode() const {
  return _mode;
}

/**
 * Returns the RGB multiplier for the first component.
 */
INLINE ColorBlendAttrib::Operand ColorBlendAttrib::
get_operand_a() const {
  return _a;
}

/**
 * Returns the RGB multiplier for the second component.
 */
INLINE ColorBlendAttrib::Operand ColorBlendAttrib::
get_operand_b() const {
  return _b;
}

/**
 * Returns the blending mode for the alpha channel.
 */
INLINE ColorBlendAttrib::Mode ColorBlendAttrib::
get_alpha_mode() const {
  return _alpha_mode;
}

/**
 * Returns the alpha multiplier for the first component.
 */
INLINE ColorBlendAttrib::Operand ColorBlendAttrib::
get_alpha_operand_a() const {
  return _alpha_a;
}

/**
 * Returns the alpha multiplier for the second component.
 */
INLINE ColorBlendAttrib::Operand ColorBlendAttrib::
get_alpha_operand_b() const {
  return _alpha_b;
}

/**
 * Returns the constant color associated with the attrib.
 */
INLINE LColor ColorBlendAttrib::
get_color() const {
  return _color;
}

/**
 * Returns true if the this attrib uses the constant color, false otherwise.
 */
INLINE bool ColorBlendAttrib::
involves_constant_color() const {
  return _involves_constant_color;
}

/**
 * Returns true if the this attrib uses the color scale attrib, false
 * otherwise.
 */
INLINE bool ColorBlendAttrib::
involves_color_scale() const {
  return _involves_color_scale;
}

/**
 * Returns true if the indicated operand uses the constant color, false
 * otherwise.
 */
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

/**
 * Returns true if the indicated operand uses the color scale attrib, false
 * otherwise.
 */
INLINE bool ColorBlendAttrib::
involves_color_scale(ColorBlendAttrib::Operand operand) {
  return (operand >= O_color_scale);
}
