// Filename: colorMaskProperty.I
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

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ColorMaskProperty::
ColorMaskProperty(int mask) :
  _mask(mask)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskProperty::all_on
//       Access: Public, Static
//  Description: This named constructor returns a ColorMaskProperty
//               whose bits are all on: that property that writes to
//               all the color planes at once (the normal case).
////////////////////////////////////////////////////////////////////
INLINE ColorMaskProperty ColorMaskProperty::
all_on() {
  return ColorMaskProperty(M_r | M_g | M_b | M_a);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskProperty::set_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ColorMaskProperty::
set_mask(int mask) {
  _mask = mask;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskProperty::get_mask
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int ColorMaskProperty::
get_mask() const {
  return _mask;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskProperty::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int ColorMaskProperty::
compare_to(const ColorMaskProperty &other) const {
  return (int)_mask - (int)other._mask;
}
