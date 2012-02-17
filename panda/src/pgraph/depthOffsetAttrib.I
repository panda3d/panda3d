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
DepthOffsetAttrib(int offset, PN_stdfloat min_value, PN_stdfloat max_value) :
  _offset(offset),
  _min_value(min_value),
  _max_value(max_value)
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

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::get_min_value
//       Access: Published
//  Description: Returns the value for the minimum (closest) depth
//               value to be stored in the buffer, in the range 0
//               .. 1.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DepthOffsetAttrib::
get_min_value() const {
  return _min_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::get_max_value
//       Access: Published
//  Description: Returns the value for the maximum (farthest) depth
//               value to be stored in the buffer, in the range 0
//               .. 1.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DepthOffsetAttrib::
get_max_value() const {
  return _max_value;
}
