// Filename: fltPackedColor.I
// Created by:  drose (25Aug00)
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

INLINE ostream &
operator << (ostream &out, const FltPackedColor &color) {
  color.output(out);
  return out;
}


////////////////////////////////////////////////////////////////////
//     Function: FltPackedColor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FltPackedColor::
FltPackedColor() {
  _a = 0;
  _b = 0;
  _g = 0;
  _r = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltPackedColor::get_color
//       Access: Public
//  Description: Returns the four-component color as a Colorf, where
//               each component is in the range [0, 1].
////////////////////////////////////////////////////////////////////
INLINE Colorf FltPackedColor::
get_color() const {
  return Colorf(_r / 255.0, _g / 255.0, _b / 255.0, _a / 255.0);
}

////////////////////////////////////////////////////////////////////
//     Function: FltPackedColor::get_rgb
//       Access: Public
//  Description: Returns the three-component color as an RGBColorf
//               (ignoring the alpha component), where each component
//               is in the range [0, 1].
////////////////////////////////////////////////////////////////////
INLINE RGBColorf FltPackedColor::
get_rgb() const {
  return RGBColorf(_r / 255.0, _g / 255.0, _b / 255.0);
}

////////////////////////////////////////////////////////////////////
//     Function: FltPackedColor::set_color
//       Access: Public
//  Description: Sets the color according to the indicated
//               four-component Colorf value (including alpha).
////////////////////////////////////////////////////////////////////
INLINE void FltPackedColor::
set_color(const Colorf &color) {
  _r = (int)floor(color[0] * 255.0);
  _g = (int)floor(color[1] * 255.0);
  _b = (int)floor(color[2] * 255.0);
  _a = (int)floor(color[3] * 255.0);
}

////////////////////////////////////////////////////////////////////
//     Function: FltPackedColor::set_rgb
//       Access: Public
//  Description: Sets the color according to the indicated
//               three-component RGBColorf value, and set the alpha to
//               1.0.
////////////////////////////////////////////////////////////////////
INLINE void FltPackedColor::
set_rgb(const RGBColorf &color) {
  _r = (int)floor(color[0] * 255.0);
  _g = (int)floor(color[1] * 255.0);
  _b = (int)floor(color[2] * 255.0);
  _a = 255;
}
