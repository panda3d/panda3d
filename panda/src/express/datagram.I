// Filename: datagram.I
// Created by:  drose (06Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Constructor
//       Access: Public
//  Description: Constructs an empty datagram.
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram() {
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Constructor
//       Access: Public
//  Description: Constructs a datagram from an existing block of data.
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram(const void *data, size_t size) {
  append_data(data, size);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Constructor
//       Access: Public
//  Description: Constructs a datagram from an existing block of data.
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram(const string &data) {
  append_data(data);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram(const Datagram &copy) :
  _data(copy._data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
operator = (const Datagram &copy) {
  _data = copy._data;
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_bool
//       Access: Public
//  Description: Adds a boolean value to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_bool(bool b) {
  add_uint8(b);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_int8
//       Access: Public
//  Description: Adds a signed 8-bit integer to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_int8(PN_int8 value) {
  append_data(&value, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_uint8
//       Access: Public
//  Description: Adds an unsigned 8-bit integer to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_uint8(PN_uint8 value) {
  append_data(&value, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_int16
//       Access: Public
//  Description: Adds a signed 16-bit integer to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_int16(PN_int16 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_int32
//       Access: Public
//  Description: Adds a signed 32-bit integer to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_int32(PN_int32 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_int64
//       Access: Public
//  Description: Adds a signed 64-bit integer to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_int64(PN_int64 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_uint16
//       Access: Public
//  Description: Adds an unsigned 16-bit integer to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_uint16(PN_uint16 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_uint32
//       Access: Public
//  Description: Adds an unsigned 32-bit integer to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_uint32(PN_uint32 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_uint64
//       Access: Public
//  Description: Adds an unsigned 64-bit integer to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_uint64(PN_uint64 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_float32
//       Access: Public
//  Description: Adds a 32-bit single-precision floating-point number
//               to the datagram.  Since this kind of float is not
//               necessarily portable across different architectures,
//               special care is required.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_float32(float value) {
  // For now, we assume the float format is portable across all
  // architectures we are concerned with.  If we come across one that
  // is different, we will have to convert.
  nassertv(sizeof(value) == 4);
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_float64
//       Access: Public
//  Description: Adds a 64-bit floating-point number to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_float64(PN_float64 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_be_int16
//       Access: Public
//  Description: Adds a signed 16-bit big-endian integer to the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_int16(PN_int16 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_be_int32
//       Access: Public
//  Description: Adds a signed 32-bit big-endian integer to the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_int32(PN_int32 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_be_int64
//       Access: Public
//  Description: Adds a signed 64-bit big-endian integer to the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_int64(PN_int64 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_be_uint16
//       Access: Public
//  Description: Adds an unsigned 16-bit big-endian integer to the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_uint16(PN_uint16 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_be_uint32
//       Access: Public
//  Description: Adds an unsigned 32-bit big-endian integer to the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_uint32(PN_uint32 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_be_uint64
//       Access: Public
//  Description: Adds an unsigned 64-bit big-endian integer to the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_uint64(PN_uint64 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_be_float32
//       Access: Public
//  Description: Adds a 32-bit single-precision big-endian
//               floating-point number to the datagram.  Since this
//               kind of float is not necessarily portable across
//               different architectures, special care is required.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_float32(float value) {
  // For now, we assume the float format is portable across all
  // architectures we are concerned with.  If we come across one that
  // is different, we will have to convert.
  nassertv(sizeof(value) == 4);
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_be_float64
//       Access: Public
//  Description: Adds a 64-bit big-endian floating-point number to the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_float64(PN_float64 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_string
//       Access: Public
//  Description: Adds a variable-length string to the datagram.  This
//               actually adds a count followed by n bytes.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_string(const string &str) {
  // The max sendable length for a string is 2^16.
  nassertv(str.length() <= (PN_uint16)0xffff);

  // Strings always are preceded by their length
  add_uint16(str.length());

  // Add the string
  append_data(str);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_string32
//       Access: Public
//  Description: Adds a variable-length string to the datagram, using
//               a 32-bit length field to allow very long strings.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_string32(const string &str) {
  // Strings always are preceded by their length
  add_uint32(str.length());

  // Add the string
  append_data(str);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_z_string
//       Access: Public
//  Description: Adds a variable-length string to the datagram, as a
//               NULL-terminated string.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_z_string(string str) {
  // We must not have any nested null characters in the string.
  size_t null_pos = str.find('\0');
  // Add the string (sans the null character).
  append_data(str.substr(0, null_pos));

  // And the null character.
  add_uint8('\0');
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::add_fixed_string
//       Access: Public
//  Description: Adds a fixed-length string to the datagram.  If the
//               string given is less than the requested size, this
//               will pad the string out with zeroes; if it is greater
//               than the requested size, this will silently truncate
//               the string.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_fixed_string(const string &str, size_t size) {
  if (str.length() < size) {
    append_data(str);
    pad_bytes(size - str.length());

  } else { // str.length() >= size
    append_data(str.substr(0, size));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::append_data
//       Access: Public
//  Description: Appends some more raw data to the end of the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
append_data(const string &data) {
  append_data(data.data(), data.length());
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::get_message
//       Access: Public
//  Description: Returns the datagram's data as a string.
////////////////////////////////////////////////////////////////////
INLINE string Datagram::
get_message() const {
  // Silly special case for gcc 3.2, which can't tolerate string(NULL, 0).
  if (_data.size() == 0) {
    return string();
  } else {
    return string((const char *)_data.p(), _data.size());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::get_data
//       Access: Public
//  Description: Returns a pointer to the beginning of the datagram's
//               data.
////////////////////////////////////////////////////////////////////
INLINE const void *Datagram::
get_data() const {
  return _data.p();
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::get_length
//       Access: Public
//  Description: Returns the number of bytes in the datagram.
////////////////////////////////////////////////////////////////////
INLINE size_t Datagram::
get_length() const {
  return _data.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Datagram::
operator == (const Datagram &other) const {
  if (_data == other._data) {
    return true;
  }
  if (_data != (uchar *)NULL && other._data != (uchar *)NULL) {
    return _data.v() == other._data.v();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Datagram::
operator != (const Datagram &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Datagram::
operator < (const Datagram &other) const {
  if (_data == other._data) {
    // Same pointers.
    return false;
  }

  if (_data != (uchar *)NULL && other._data != (uchar *)NULL) {
    // Different pointers, neither NULL.
    return _data.v() < other._data.v();
  }

  // One of the pointers is NULL, but not the other one.
  return _data.size() < other._data.size();
}
