// Filename: rescaleNormalAttrib.I
// Created by:  drose (30Dec04)
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
//     Function: RescaleNormalAttrib::Constructor
//       Access: Private
//  Description: Use RescaleNormalAttrib::make() to construct a new
//               RescaleNormalAttrib object.
////////////////////////////////////////////////////////////////////
INLINE RescaleNormalAttrib::
RescaleNormalAttrib(RescaleNormalAttrib::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RescaleNormalAttrib::get_mode
//       Access: Published
//  Description: Returns the render mode.
////////////////////////////////////////////////////////////////////
INLINE RescaleNormalAttrib::Mode RescaleNormalAttrib::
get_mode() const {
  return _mode;
}
