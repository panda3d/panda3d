// Filename: stencilAttribute.I
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
//     Function: StencilAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilAttribute::
StencilAttribute() :
  _value(StencilProperty::M_none, StencilProperty::A_keep)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StencilAttribute::
set_mode(StencilProperty::Mode mode) {
  _value.set_mode(mode);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::Mode StencilAttribute::
get_mode() const {
  return _value.get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::set_action
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StencilAttribute::
set_action(StencilProperty::Action action) {
  _value.set_action(action);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttribute::get_action
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::Action StencilAttribute::
get_action() const {
  return _value.get_action();
}
