// Filename: littleEndian.I
// Created by:  drose (09Feb00)
// 
////////////////////////////////////////////////////////////////////


#ifdef IS_BIG_ENDIAN

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::Constructor
//       Access: Public
//  Description: Reverses the bytes in the indicated string.
////////////////////////////////////////////////////////////////////
INLINE LittleEndian::
LittleEndian(const string &string) {
  reverse_assign(string.data(), string.length());
}

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::Constructor
//       Access: Public
//  Description: Reverses the bytes in the indicated string.
////////////////////////////////////////////////////////////////////
INLINE LittleEndian::
LittleEndian(const char *data, size_t length) {
  reverse_assign(data, length);
}

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::Copy Constructor
//       Access: Public
//  Description: Since this is assigned from another LittleEndian,
//               presumably already reversed, this does *not* reverse
//               the byte order.
////////////////////////////////////////////////////////////////////
INLINE LittleEndian::
LittleEndian(const LittleEndian &other) : _str(other._str) {
}

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::String Assignment Operator
//       Access: Public
//  Description: Reverses the bytes in the indicated string.
////////////////////////////////////////////////////////////////////
INLINE void LittleEndian::
operator =(const string &string) {
  reverse_assign(string.data(), string.length());
}

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::Copy Assignment Operator
//       Access: Public
//  Description: Since this is assigned from another LittleEndian,
//               presumably already reversed, this does *not* reverse
//               the byte order.
////////////////////////////////////////////////////////////////////
INLINE void LittleEndian::
operator =(const LittleEndian &other) {
  _str = other._str;
}

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::string Typecast Operator
//       Access: Public
//  Description: Returns the data as a string.
////////////////////////////////////////////////////////////////////
INLINE LittleEndian::
operator const string &() const {
  return _str;
}

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::data
//       Access: Public
//  Description: Returns the data in the string.
////////////////////////////////////////////////////////////////////
INLINE const char *LittleEndian::
data() const {
  return _str.data();
}

////////////////////////////////////////////////////////////////////
//     Function: LittleEndian::length
//       Access: Public
//  Description: Returns the number of bytes in the string.
////////////////////////////////////////////////////////////////////
INLINE size_t LittleEndian::
length() const {
  return _str.length();
}

#endif // IS_BIG_ENDIAN
