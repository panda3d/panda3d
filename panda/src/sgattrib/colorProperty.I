// Filename: colorProperty.I
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: ColorProperty::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ColorProperty::
ColorProperty() : 
  _real(false) 
{ 
}

////////////////////////////////////////////////////////////////////
//     Function: ColorProperty::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ColorProperty::
ColorProperty(const Colorf &color) : 
  _color(color), 
  _real(true) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorProperty::is_real
//       Access: Public
//  Description: Returns true if the color is a real color, false if
//               it is the uncolor.
////////////////////////////////////////////////////////////////////
INLINE bool ColorProperty::
is_real() const {
  return _real;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorProperty::get_color
//       Access: Public
//  Description: Returns the four-component color.  It is only valid
//               to call this method if is_real() returns true.
////////////////////////////////////////////////////////////////////
INLINE Colorf ColorProperty::
get_color() const {
  nassertr(is_real(), Colorf(0.0, 0.0, 0.0, 0.0));
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorProperty::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int ColorProperty::
compare_to(const ColorProperty &other) const {
  if (_real != other._real) {
    return (int)_real - (int)other._real;
  }
  if (_real) {
    return _color.compare_to(other._color);
  }
  return 0;
}
