// Filename: bigEndian.I
// Created by:  drose (23Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifdef IS_LITTLE_ENDIAN

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::Constructor
//       Access: Public
//  Description: Reverses the bytes in the indicated string.
////////////////////////////////////////////////////////////////////
INLINE BigEndian::
BigEndian(const string &string) {
  reverse_assign(string.data(), string.length());
}

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::Constructor
//       Access: Public
//  Description: Reverses the bytes in the indicated string.
////////////////////////////////////////////////////////////////////
INLINE BigEndian::
BigEndian(const char *data, size_t length) {
  reverse_assign(data, length);
}

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::Copy Constructor
//       Access: Public
//  Description: Since this is assigned from another BigEndian,
//               presumably already reversed, this does *not* reverse
//               the byte order.
////////////////////////////////////////////////////////////////////
INLINE BigEndian::
BigEndian(const BigEndian &other) : _str(other._str) {
}

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::String Assignment Operator
//       Access: Public
//  Description: Reverses the bytes in the indicated string.
////////////////////////////////////////////////////////////////////
INLINE void BigEndian::
operator =(const string &string) {
  reverse_assign(string.data(), string.length());
}

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::Copy Assignment Operator
//       Access: Public
//  Description: Since this is assigned from another BigEndian,
//               presumably already reversed, this does *not* reverse
//               the byte order.
////////////////////////////////////////////////////////////////////
INLINE void BigEndian::
operator =(const BigEndian &other) {
  _str = other._str;
}

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::string Typecast Operator
//       Access: Public
//  Description: Returns the data as a string.
////////////////////////////////////////////////////////////////////
INLINE BigEndian::
operator const string &() const {
  return _str;
}

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::data
//       Access: Public
//  Description: Returns the data in the string.
////////////////////////////////////////////////////////////////////
INLINE const char *BigEndian::
data() const {
  return _str.data();
}

////////////////////////////////////////////////////////////////////
//     Function: BigEndian::length
//       Access: Public
//  Description: Returns the number of bytes in the string.
////////////////////////////////////////////////////////////////////
INLINE size_t BigEndian::
length() const {
  return _str.length();
}

#endif // IS_LITTLE_ENDIAN
