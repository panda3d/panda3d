// Filename: transparencyAttrib.I
// Created by:  drose (28Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
