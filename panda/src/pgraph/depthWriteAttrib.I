// Filename: depthWriteAttrib.I
// Created by:  drose (04Mar02)
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
//     Function: DepthWriteAttrib::Constructor
//       Access: Private
//  Description: Use DepthWriteAttrib::make() to construct a new
//               DepthWriteAttrib object.
////////////////////////////////////////////////////////////////////
INLINE DepthWriteAttrib::
DepthWriteAttrib(DepthWriteAttrib::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttrib::get_mode
//       Access: Published
//  Description: Returns the depth write mode.
////////////////////////////////////////////////////////////////////
INLINE DepthWriteAttrib::Mode DepthWriteAttrib::
get_mode() const {
  return _mode;
}
