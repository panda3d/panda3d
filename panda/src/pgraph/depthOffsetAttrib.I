// Filename: depthOffsetAttrib.I
// Created by:  drose (14Mar02)
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
//     Function: DepthOffsetAttrib::Constructor
//       Access: Private
//  Description: Use DepthOffsetAttrib::make() to construct a new
//               DepthOffsetAttrib object.
////////////////////////////////////////////////////////////////////
INLINE DepthOffsetAttrib::
DepthOffsetAttrib(int offset) :
  _offset(offset)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::get_offset
//       Access: Published
//  Description: Returns the depth offset represented by this attrib.
////////////////////////////////////////////////////////////////////
INLINE int DepthOffsetAttrib::
get_offset() const {
  return _offset;
}
