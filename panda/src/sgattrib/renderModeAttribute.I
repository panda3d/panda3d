// Filename: renderModeAttribute.I
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
//     Function: RenderModeAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderModeAttribute::
RenderModeAttribute() :
  _value(RenderModeProperty::M_filled, 1.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderModeAttribute::
set_mode(RenderModeProperty::Mode mode) {
  _value.set_mode(mode);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderModeProperty::Mode RenderModeAttribute::
get_mode() const {
  return _value.get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::set_line_width
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderModeAttribute::
set_line_width(double line_width) {
  _value.set_line_width(line_width);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttribute::get_line_width
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE double RenderModeAttribute::
get_line_width() const {
  return _value.get_line_width();
}
