// Filename: nativeNumericData.I
// Created by:  drose (09May01)
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
//     Function: NativeNumericData::Constructor
//       Access: Public
//  Description: This constructor accepts the address of a numeric
//               variable, and its sizeof.
////////////////////////////////////////////////////////////////////
INLINE NativeNumericData::
NativeNumericData(const void *data, size_t) :
  _source(data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NativeNumericData::Constructor
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
INLINE NativeNumericData::
NativeNumericData(const void *data, size_t start, size_t) {
  _source = (void *)((const char *)data + start);
}

////////////////////////////////////////////////////////////////////
//     Function: NativeNumericData::store_value
//       Access: Public
//  Description: Copies the data, with byte reversal if appropriate,
//               into the indicated numeric variable, whose address
//               and sizeof are given.
////////////////////////////////////////////////////////////////////
INLINE void NativeNumericData::
store_value(void *dest, size_t length) const {
  memcpy(dest, _source, length);
}

////////////////////////////////////////////////////////////////////
//     Function: NativeNumericData::get_data
//       Access: Public
//  Description: Returns the pointer to the first byte of the data,
//               either reversed or nonreversed, as appropriate.
////////////////////////////////////////////////////////////////////
INLINE const void *NativeNumericData::
get_data() const {
  return _source;
}
