/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltPackedColor.I
 * @author drose
 * @date 2000-08-25
 */

INLINE std::ostream &
operator << (std::ostream &out, const FltPackedColor &color) {
  color.output(out);
  return out;
}


/**
 *
 */
INLINE FltPackedColor::
FltPackedColor() {
  _a = 0;
  _b = 0;
  _g = 0;
  _r = 0;
}

/**
 * Returns the four-component color as a LColor, where each component is in
 * the range [0, 1].
 */
INLINE LColor FltPackedColor::
get_color() const {
  return LColor(_r / 255.0, _g / 255.0, _b / 255.0, _a / 255.0);
}

/**
 * Returns the three-component color as an LRGBColor (ignoring the alpha
 * component), where each component is in the range [0, 1].
 */
INLINE LRGBColor FltPackedColor::
get_rgb() const {
  return LRGBColor(_r / 255.0, _g / 255.0, _b / 255.0);
}

/**
 * Sets the color according to the indicated four-component LColor value
 * (including alpha).
 */
INLINE void FltPackedColor::
set_color(const LColor &color) {
  _r = (int)floor(color[0] * 255.0);
  _g = (int)floor(color[1] * 255.0);
  _b = (int)floor(color[2] * 255.0);
  _a = (int)floor(color[3] * 255.0);
}

/**
 * Sets the color according to the indicated three-component LRGBColor value,
 * and set the alpha to 1.0.
 */
INLINE void FltPackedColor::
set_rgb(const LRGBColor &color) {
  _r = (int)floor(color[0] * 255.0);
  _g = (int)floor(color[1] * 255.0);
  _b = (int)floor(color[2] * 255.0);
  _a = 255;
}
