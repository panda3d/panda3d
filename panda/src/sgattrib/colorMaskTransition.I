// Filename: colorMaskTransition.I
// Created by:  drose (23Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
