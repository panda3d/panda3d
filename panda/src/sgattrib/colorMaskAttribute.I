// Filename: colorMaskAttribute.I
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ColorMaskAttribute::
ColorMaskAttribute() :
  _value(ColorMaskProperty::all_on())
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::set_mask
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ColorMaskAttribute::
set_mask(int mask) {
  _value.set_mask(mask);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::get_mask
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int ColorMaskAttribute::
get_mask() const {
  return _value.get_mask();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::is_write_r
//       Access: Public
//  Description: Returns true if the red component is enabled for
//               writing.
////////////////////////////////////////////////////////////////////
INLINE bool ColorMaskAttribute::
is_write_r() const {
  return (_value.get_mask() & ColorMaskProperty::M_r) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::is_write_g
//       Access: Public
//  Description: Returns true if the green component is enabled for
//               writing.
////////////////////////////////////////////////////////////////////
INLINE bool ColorMaskAttribute::
is_write_g() const {
  return (_value.get_mask() & ColorMaskProperty::M_g) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::is_write_b
//       Access: Public
//  Description: Returns true if the blue component is enabled for
//               writing.
////////////////////////////////////////////////////////////////////
INLINE bool ColorMaskAttribute::
is_write_b() const {
  return (_value.get_mask() & ColorMaskProperty::M_b) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskAttribute::is_write_a
//       Access: Public
//  Description: Returns true if the alpha component is enabled for
//               writing.
////////////////////////////////////////////////////////////////////
INLINE bool ColorMaskAttribute::
is_write_a() const {
  return (_value.get_mask() & ColorMaskProperty::M_a) != 0;
}
