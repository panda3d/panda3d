// Filename: antialiasAttrib.I
// Created by:  drose (26Jan05)
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
//     Function: AntialiasAttrib::Constructor
//       Access: Private
//  Description: Use AntialiasAttrib::make() to construct a new
//               AntialiasAttrib object.
////////////////////////////////////////////////////////////////////
INLINE AntialiasAttrib::
AntialiasAttrib(unsigned short mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::get_mode
//       Access: Published
//  Description: Returns the render mode.
////////////////////////////////////////////////////////////////////
INLINE unsigned short AntialiasAttrib::
get_mode() const {
  return _mode;
}
