// Filename: stringStream.cxx
// Created by:  drose (03Jul07)
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

#include "stringStream.h"

////////////////////////////////////////////////////////////////////
//     Function: StringStream::set_data
//       Access: Public
//  Description: Replaces the contents of the data stream.  This
//               implicitly reseeks to 0.
////////////////////////////////////////////////////////////////////
void StringStream::
set_data(const unsigned char *data, size_t size) {
  _buf.clear();
  pvector<unsigned char> pv;
  pv.insert(pv.end(), data, data + size);
  _buf.swap_data(pv);
}
