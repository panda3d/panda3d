// Filename: renderModeTransition.I
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
//     Function: RenderModeTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderModeTransition::
RenderModeTransition(RenderModeProperty::Mode mode, double line_width) :
  _value(mode, line_width)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderModeTransition::
set_mode(RenderModeProperty::Mode mode) {
  _value.set_mode(mode);
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderModeProperty::Mode RenderModeTransition::
get_mode() const {
  return _value.get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::set_line_width
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderModeTransition::
set_line_width(double line_width) {
  _value.set_line_width(line_width);
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeTransition::get_line_width
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE double RenderModeTransition::
get_line_width() const {
  return _value.get_line_width();
}
