// Filename: colorBlendTransition.I
// Created by:  drose (22Mar00)
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
//     Function: ColorBlendTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ColorBlendTransition::
ColorBlendTransition(ColorBlendProperty::Mode mode) :
  _value(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ColorBlendTransition::
set_mode(ColorBlendProperty::Mode mode) {
  _value.set_mode(mode);
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendTransition::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ColorBlendProperty::Mode ColorBlendTransition::
get_mode() const {
  return _value.get_mode();
}
