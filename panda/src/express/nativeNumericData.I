// Filename: nativeNumericData.I
// Created by:  drose (09May01)
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
INLINE NativeNumericData::
NativeNumericData(const string &data, size_t start, size_t) {
  _source = (void *)(data.data() + start);
}

////////////////////////////////////////////////////////////////////
//     Function: NativeNumericData::store_value
//       Access: Public
//  Description: Copies the data, with byte reversal if appropriate,
//               into the indicated numeric variable, whose address
//               and sizeof are given.
////////////////////////////////////////////////////////////////////
INLINE void NativeNumericData::
store_value(void *dest, size_t length) {
  memcpy(dest, _source, length);
}

////////////////////////////////////////////////////////////////////
//     Function: NativeNumericData::append_to
//       Access: Public
//  Description: Packs the numeric data, with byte reversal if
//               appropriate, onto the end of the given string.
////////////////////////////////////////////////////////////////////
INLINE void NativeNumericData::
append_to(string &dest, size_t length) {
  dest.append((const char *)_source, length);
}
