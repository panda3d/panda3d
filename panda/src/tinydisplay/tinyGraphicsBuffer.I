// Filename: tinyGraphicsBuffer.I
// Created by:  drose (08Aug08)
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
//     Function: TinyGraphicsBuffer::get_frame_buffer
//       Access: Public
//  Description: Returns a direct pointer to the frame buffer used in
//               this object.
////////////////////////////////////////////////////////////////////
ZBuffer *TinyGraphicsBuffer::
get_frame_buffer() {
  return _frame_buffer;
}
