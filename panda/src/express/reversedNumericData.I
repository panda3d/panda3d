// Filename: reversedNumericData.I
// Created by:  drose (09May01)
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
//  Description: This constructor accepts a string containing a packed
//               numeric value, the offset within the string at which
//               the numeric value starts, and the size of the numeric
//               value.
//
//               It is essential that the string not be destructed or
//               modified as long as the NumericData object remains;
//               it may just store a pointer into that string's
//               internal buffer.
////////////////////////////////////////////////////////////////////
INLINE ReversedNumericData::
ReversedNumericData(const string &data, size_t start, size_t length) {
  reverse_assign(data.data() + start, length);
}

////////////////////////////////////////////////////////////////////
//     Function: ReversedNumericData::store_value
//       Access: Public
//  Description: Copies the data, with byte reversal if appropriate,
//               into the indicated numeric variable, whose address
//               and sizeof are given.
////////////////////////////////////////////////////////////////////
INLINE void ReversedNumericData::
store_value(void *dest, size_t length) {
  memcpy(dest, _data, length);
}

////////////////////////////////////////////////////////////////////
//     Function: ReversedNumericData::append_to
//       Access: Public
//  Description: Packs the numeric data, with byte reversal if
//               appropriate, onto the end of the given string.
////////////////////////////////////////////////////////////////////
INLINE void ReversedNumericData::
append_to(string &dest, size_t length) {
  dest.append((const char *)_data, length);
}
