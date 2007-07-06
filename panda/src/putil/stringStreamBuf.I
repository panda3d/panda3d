// Filename: stringStreamBuf.I
// Created by:  drose (03Jul07)
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
