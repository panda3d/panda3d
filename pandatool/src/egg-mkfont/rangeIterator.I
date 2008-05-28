// Filename: rangeIterator.I
// Created by:  drose (07Sep03)
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
//     Function: RangeIterator::get_code
//       Access: Public
//  Description: Returns the current Unicode value represented by the
//               iterator, or -1 if the iterator has reached the end.
////////////////////////////////////////////////////////////////////
INLINE int RangeIterator::
get_code() const {
  return _code;
}

////////////////////////////////////////////////////////////////////
//     Function: RangeIterator::eof
//       Access: Public
//  Description: Returns true if all the code have been retrieved,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RangeIterator::
eof() const {
  return (_it == _desc._range_list.end());
}
