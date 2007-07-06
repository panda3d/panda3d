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
//     Function: StringStreamBuf::get_data_size
//       Access: Public
//  Description: Returns the number of characters remaining in the
//               internal data buffer, not counting data which might
//               still be in the iostream buffer.
////////////////////////////////////////////////////////////////////
INLINE size_t StringStreamBuf::
get_data_size() const {
  return _data.size() - _gpos;
}
