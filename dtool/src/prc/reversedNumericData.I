// Filename: reversedNumericData.I
// Created by:  drose (09May01)
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
//     Function: ReversedNumericData::Constructor
//       Access: Public
//  Description: This constructor accepts the address of a numeric
//               variable, and its sizeof.
////////////////////////////////////////////////////////////////////
INLINE ReversedNumericData::
ReversedNumericData(const void *data, size_t length) {
  reverse_assign((const char *)data, length);
}

////////////////////////////////////////////////////////////////////
//     Function: ReversedNumericData::Constructor
//       Access: Public
//  Description: This constructor accepts a pointer to a data array
//               containing a packed numeric value, the offset within
//               the array at which the numeric value starts, and the
//               size of the numeric value.
//
//               It is essential that the array not be destructed or
//               modified as long as the NumericData object remains;
//               it may just store a pointer into that string's
//               internal buffer.
////////////////////////////////////////////////////////////////////
INLINE ReversedNumericData::
ReversedNumericData(const void *data, size_t start, size_t length) {
  reverse_assign((const char *)data + start, length);
}

////////////////////////////////////////////////////////////////////
//     Function: ReversedNumericData::store_value
//       Access: Public
//  Description: Copies the data, with byte reversal if appropriate,
//               into the indicated numeric variable, whose address
//               and sizeof are given.
////////////////////////////////////////////////////////////////////
INLINE void ReversedNumericData::
store_value(void *dest, size_t length) const {
  memcpy(dest, _data, length);
}

////////////////////////////////////////////////////////////////////
//     Function: ReversedNumericData::get_data
//       Access: Public
//  Description: Returns the pointer to the first byte of the data,
//               either reversed or nonreversed, as appropriate.
////////////////////////////////////////////////////////////////////
INLINE const void *ReversedNumericData::
get_data() const {
  return _data;
}
