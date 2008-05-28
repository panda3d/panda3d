// Filename: shadeModelAttrib.I
// Created by:  drose (14Mar05)
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
//     Function: ShadeModelAttrib::Constructor
//       Access: Private
//  Description: Use ShadeModelAttrib::make() to construct a new
//               ShadeModelAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ShadeModelAttrib::
ShadeModelAttrib(ShadeModelAttrib::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::get_mode
//       Access: Published
//  Description: Returns the shade mode.
////////////////////////////////////////////////////////////////////
INLINE ShadeModelAttrib::Mode ShadeModelAttrib::
get_mode() const {
  return _mode;
}
