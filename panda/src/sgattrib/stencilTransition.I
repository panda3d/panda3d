// Filename: stencilTransition.I
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
//     Function: StencilTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilTransition::
StencilTransition(StencilProperty::Mode mode,
                  StencilProperty::Action action) :
  _value(mode, action)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StencilTransition::
set_mode(StencilProperty::Mode mode) {
  _value.set_mode(mode);
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::Mode StencilTransition::
get_mode() const {
  return _value.get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::set_action
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StencilTransition::
set_action(StencilProperty::Action action) {
  _value.set_action(action);
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::get_action
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::Action StencilTransition::
get_action() const {
  return _value.get_action();
}
