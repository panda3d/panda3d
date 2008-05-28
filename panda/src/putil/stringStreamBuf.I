// Filename: stringStreamBuf.I
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


////////////////////////////////////////////////////////////////////
//     Function: StringStreamBuf::swap_data
//       Access: Public
//  Description: Swaps the indicated buffer for the contents of the
//               internal buffer.  Does not affect the ppos or gpos,
//               or the iostream buffer.
////////////////////////////////////////////////////////////////////
INLINE void StringStreamBuf::
swap_data(pvector<unsigned char> &data) {
  _data.swap(data);
}

////////////////////////////////////////////////////////////////////
//     Function: StringStreamBuf::get_data
//       Access: Public
//  Description: Returns a reference to the contents of the internal
//               buffer, without any of the iostream buffer.
////////////////////////////////////////////////////////////////////
INLINE const pvector<unsigned char> &StringStreamBuf::
get_data() const {
  return _data;
}
