// Filename: shadeModelAttrib.I
// Created by:  drose (14Mar05)
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
