// Filename: stencilProperty.I
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
//     Function: StencilProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::
StencilProperty(StencilProperty::Mode mode, StencilProperty::Action action) :
  _mode(mode),
  _action(action)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StencilProperty::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StencilProperty::
set_mode(StencilProperty::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilProperty::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::Mode StencilProperty::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilProperty::set_action
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StencilProperty::
set_action(StencilProperty::Action action) {
  _action = action;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilProperty::get_action
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::Action StencilProperty::
get_action() const {
  return _action;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilProperty::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int StencilProperty::
compare_to(const StencilProperty &other) const {
  if (_mode != other._mode) {
    return (int)_mode - (int)other._mode;
  }
  return (int)_action - (int)other._action;
}
