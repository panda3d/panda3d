// Filename: colorBlendAttrib.I
// Created by:  drose (29Mar02)
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
//     Function: ColorBlendAttrib::Constructor
//       Access: Private
//  Description: Use ColorBlendAttrib::make() to construct a new
//               ColorBlendAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorBlendAttrib::
ColorBlendAttrib(ColorBlendAttrib::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::get_mode
//       Access: Published
//  Description: Returns the colorBlend mode.
////////////////////////////////////////////////////////////////////
INLINE ColorBlendAttrib::Mode ColorBlendAttrib::
get_mode() const {
  return _mode;
}
