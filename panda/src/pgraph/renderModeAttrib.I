// Filename: renderModeAttrib.I
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::Constructor
//       Access: Private
//  Description: Use RenderModeAttrib::make() to construct a new
//               RenderModeAttrib object.
////////////////////////////////////////////////////////////////////
INLINE RenderModeAttrib::
RenderModeAttrib(RenderModeAttrib::Mode mode, float line_width) :
  _mode(mode),
  _line_width(line_width)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::get_mode
//       Access: Published
//  Description: Returns the render mode.
////////////////////////////////////////////////////////////////////
INLINE RenderModeAttrib::Mode RenderModeAttrib::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::get_line_width
//       Access: Published
//  Description: Returns the line width.  This is only relevant when
//               the mode is M_wireframe.
////////////////////////////////////////////////////////////////////
INLINE float RenderModeAttrib::
get_line_width() const {
  return _line_width;
}
