/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorSpace.h
 * @author rdb
 * @date 2014-06-02
 */

#ifndef COLORSPACE_H
#define COLORSPACE_H

#include "pandabase.h"

#include "typedef.h"

BEGIN_PUBLISH

enum ColorSpace {
  // This value is not a color space, but is used to indicate that a color
  // space has not been specified.
  CS_unspecified = 0,

  // CS_linear is not a color space per se, but represents the working color
  // space of graphics APIs, which is linearized.  Since the conversion from
  // sRGB to linear is defined, one could posit that it has the ITU-R BT.709
  // primaries, but this isn't meaningful as modern graphics APIs do not
  // perform color management.  All colors in Panda3D are linear unless
  // otherwise specified.
  CS_linear,

  // This is the standard, gamma-2.2-corrected sRGB color space, as used by
  // the majority of image formats.
  CS_sRGB,

  // This is a 16-bit encoded linear color space capable of encoding color
  // values in the -0.5...7.4999 range.
  CS_scRGB,
};

EXPCL_PANDA_PUTIL ColorSpace parse_color_space_string(const std::string &str);
EXPCL_PANDA_PUTIL std::string format_color_space(ColorSpace cs);

END_PUBLISH

EXPCL_PANDA_PUTIL std::ostream &operator << (std::ostream &out, ColorSpace cs);
EXPCL_PANDA_PUTIL std::istream &operator >> (std::istream &in, ColorSpace &cs);

#endif
