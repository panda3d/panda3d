// Filename: transparencyAttrib.I
// Created by:  drose (28Feb02)
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
//     Function: TransparencyAttrib::Constructor
//       Access: Private
//  Description: Use TransparencyAttrib::make() to construct a new
//               TransparencyAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TransparencyAttrib::
TransparencyAttrib(TransparencyAttrib::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyAttrib::get_mode
//       Access: Published
//  Description: Returns the transparency mode.
////////////////////////////////////////////////////////////////////
INLINE TransparencyAttrib::Mode TransparencyAttrib::
get_mode() const {
  return _mode;
}
