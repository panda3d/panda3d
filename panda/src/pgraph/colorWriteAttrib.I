// Filename: colorWriteAttrib.I
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
//     Function: ColorWriteAttrib::Constructor
//       Access: Private
//  Description: Use ColorWriteAttrib::make() to construct a new
//               ColorWriteAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorWriteAttrib::
ColorWriteAttrib(unsigned int channels) :
  _channels(channels)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorWriteAttrib::get_channels
//       Access: Published
//  Description: Returns the mask of color channels that are enabled
//               by this attrib.
////////////////////////////////////////////////////////////////////
INLINE unsigned int ColorWriteAttrib::
get_channels() const {
  return _channels;
}
