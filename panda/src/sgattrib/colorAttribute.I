// Filename: colorAttribute.I
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ColorAttribute::
ColorAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::set_on
//       Access: Public
//  Description: Changes the ColorAttribute to turn on the particular
//               color.
////////////////////////////////////////////////////////////////////
INLINE void ColorAttribute::
set_on(const Colorf &color) {
  _value = ColorProperty(color);
  OnOffAttribute::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::set_on
//       Access: Public
//  Description: Changes the ColorAttribute to turn on the particular
//               color.
////////////////////////////////////////////////////////////////////
INLINE void ColorAttribute::
set_on(float r, float g, float b, float a) {
  set_on(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::set_uncolor
//       Access: Public
//  Description: Changes the ColorAttribute to turn on uncolor.
////////////////////////////////////////////////////////////////////
INLINE void ColorAttribute::
set_uncolor() {
  _value = ColorProperty();
}
 
////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::is_real
//       Access: Public
//  Description: Returns true if the ColorAttribute represents a real
//               color, false if it represents the uncolor.  It is
//               only valid to call this if is_on() has returned true.
////////////////////////////////////////////////////////////////////
INLINE bool ColorAttribute::
is_real() const {
  nassertr(is_on(), false);
  return _value.is_real();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorAttribute::get_color
//       Access: Public
//  Description: Returns the color that the ColorAttribute
//               represents.  It is only valid to call this if is_on()
//               and is_real() have both returned true.
////////////////////////////////////////////////////////////////////
INLINE Colorf ColorAttribute::
get_color() const {
  nassertr(is_on(), false);
  return _value.get_color();
}
