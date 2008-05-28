// Filename: rescaleNormalAttrib.I
// Created by:  drose (30Dec04)
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
