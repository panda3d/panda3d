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
ColorMaskTransition() :
  _value(ColorMaskProperty::all_on())
{
}

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

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::is_write_r
//       Access: Public
//  Description: Returns true if the red component is enabled for
//               writing.
////////////////////////////////////////////////////////////////////
INLINE bool ColorMaskTransition::
is_write_r() const {
  return (_value.get_mask() & ColorMaskProperty::M_r) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::is_write_g
//       Access: Public
//  Description: Returns true if the green component is enabled for
//               writing.
////////////////////////////////////////////////////////////////////
INLINE bool ColorMaskTransition::
is_write_g() const {
  return (_value.get_mask() & ColorMaskProperty::M_g) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::is_write_b
//       Access: Public
//  Description: Returns true if the blue component is enabled for
//               writing.
////////////////////////////////////////////////////////////////////
INLINE bool ColorMaskTransition::
is_write_b() const {
  return (_value.get_mask() & ColorMaskProperty::M_b) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskTransition::is_write_a
//       Access: Public
//  Description: Returns true if the alpha component is enabled for
//               writing.
////////////////////////////////////////////////////////////////////
INLINE bool ColorMaskTransition::
is_write_a() const {
  return (_value.get_mask() & ColorMaskProperty::M_a) != 0;
}
