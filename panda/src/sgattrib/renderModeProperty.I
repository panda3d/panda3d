// Filename: renderModeProperty.I
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
//     Function: RenderModeProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderModeProperty::
RenderModeProperty(Mode mode, double line_width) :
  _mode(mode),
  _line_width(line_width)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeProperty::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderModeProperty::
set_mode(RenderModeProperty::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeProperty::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderModeProperty::Mode RenderModeProperty::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeProperty::set_line_width
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderModeProperty::
set_line_width(double line_width) {
  _line_width = line_width;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeProperty::get_line_width
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE double RenderModeProperty::
get_line_width() const {
  return _line_width;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeProperty::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int RenderModeProperty::
compare_to(const RenderModeProperty &other) const {
  if (_mode != other._mode) {
    return (int)_mode - (int)other._mode;
  }
  if (_line_width != other._line_width) {
    return _line_width < other._line_width ? -1 : 1;
  }
  return 0;
}

