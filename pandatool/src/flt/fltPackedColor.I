// Filename: fltPackedColor.I
// Created by:  drose (25Aug00)
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
