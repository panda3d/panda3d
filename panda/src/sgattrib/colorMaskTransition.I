// Filename: colorMaskTransition.I
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ColorMaskTransition::
ColorMaskTransition(int mask) :
  _value(mask)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::all_on
//       Access: Public, Static
//  Description: This named constructor returns a ColorMaskTransition
//               whose bits are all on: that transition that writes to
//               all the color planes at once (the normal case).
////////////////////////////////////////////////////////////////////
INLINE ColorMaskTransition ColorMaskTransition::
all_on() {
  ColorMaskTransition t(0);
  t._value = ColorMaskProperty::all_on();
  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::set_mask
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ColorMaskTransition::
set_mask(int mask) {
  _value.set_mask(mask);
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::get_mask
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int ColorMaskTransition::
get_mask() const {
  return _value.get_mask();
}
