// Filename: depthOffsetAttrib.I
// Created by:  drose (14Mar02)
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
