// Filename: datagram.I
// Created by:  drose (06Jun00)
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
//     Function: Datagram::Constructor
//       Access: Public
//  Description: Constructs an empty datagram.
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram() :
#ifdef STDFLOAT_DOUBLE
  _stdfloat_double(true)
#else
  _stdfloat_double(false)
#endif
{
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Constructor
//       Access: Public
//  Description: Constructs a datagram from an existing block of data.
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram(const void *data, size_t size) :
#ifdef STDFLOAT_DOUBLE
  _stdfloat_double(true)
#else
  _stdfloat_double(false)
#endif
{
  append_data(data, size);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Constructor
//       Access: Public
//  Description: Constructs a datagram from an existing block of data.
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram(const string &data) :
#ifdef STDFLOAT_DOUBLE
  _stdfloat_double(true)
#else
  _stdfloat_double(false)
#endif
{
  append_data(data);
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram(const Datagram &copy) :
  _data(copy._data),
  _stdfloat_double(copy._stdfloat_double)
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
  _stdfloat_double = copy._stdfloat_double;
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: Datagram::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Datagram::
Datagram(Datagram &&from) NOEXCEPT :
  _data(move(from._data)),
  _stdfloat_double(from._stdfloat_double)
{
}
#endif  // USE_MOVE_SEMANTICS

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: Datagram::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
operator = (Datagram &&from) NOEXCEPT {
  _data = move(from._data);
  _stdfloat_double = from._stdfloat_double;
}
#endif  // USE_MOVE_SEMANTICS

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
add_float32(PN_float32 value) {
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
//     Function: Datagram::add_stdfloat
//       Access: Public
//  Description: Adds either a 32-bit or a 64-bit floating-point
//               number, according to set_stdfloat_double().
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_stdfloat(PN_stdfloat value) {
  if (_stdfloat_double) {
    add_float64(value);
  } else {
    add_float32(value);
  }
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
//               floating-point number to the datagram.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
add_be_float32(PN_float32 value) {
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
//     Function: Datagram::set_array
//       Access: Public
//  Description: Replaces the data in the Datagram with the data in
//               the indicated PTA_uchar.  This is assignment by
//               reference: subsequent changes to the Datagram will
//               also change the source PTA_uchar.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
set_array(PTA_uchar data) {
  _data = data;
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::copy_array
//       Access: Public
//  Description: Replaces the data in the Datagram with a copy of the
//               data in the indicated CPTA_uchar.  Unlike
//               set_array(), a complete copy is made of the data;
//               subsequent changes to the Datagram will *not* change
//               the source CPTA_uchar.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
copy_array(CPTA_uchar data) {
  _data.clear();
  _data.v() = data.v();
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::get_array
//       Access: Public
//  Description: Returns a const pointer to the actual data in
//               the Datagram.
////////////////////////////////////////////////////////////////////
INLINE CPTA_uchar Datagram::
get_array() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::modify_array
//       Access: Public
//  Description: Returns a modifiable pointer to the actual data in
//               the Datagram.
////////////////////////////////////////////////////////////////////
INLINE PTA_uchar Datagram::
modify_array() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::set_stdfloat_double
//       Access: Public
//  Description: Changes the stdfloat_double flag, which defines the
//               operation performed by add_stdfloat() and
//               DatagramIterator::get_stdfloat().  When this is true,
//               add_stdfloat() adds a 64-bit floating-point number;
//               when it is false, it adds a 32-bit floating-point
//               number.  The default is based on the STDFLOAT_DOUBLE
//               compilation flag.
////////////////////////////////////////////////////////////////////
INLINE void Datagram::
set_stdfloat_double(bool stdfloat_double) {
  _stdfloat_double = stdfloat_double;
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::get_stdfloat_double
//       Access: Public
//  Description: Returns the stdfloat_double flag.  See
//               set_stdfloat_double().
////////////////////////////////////////////////////////////////////
INLINE bool Datagram::
get_stdfloat_double() const {
  return _stdfloat_double;
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

INLINE void
generic_write_datagram(Datagram &dest, bool value) {
  dest.add_bool(value);
}

INLINE void
generic_write_datagram(Datagram &dest, int value) {
  dest.add_int32(value);
}

INLINE void
generic_write_datagram(Datagram &dest, float value) {
  dest.add_float32(value);
}

INLINE void
generic_write_datagram(Datagram &dest, double value) {
  dest.add_float64(value);
}

INLINE void
generic_write_datagram(Datagram &dest, const string &value) {
  dest.add_string(value);
}

INLINE void
generic_write_datagram(Datagram &dest, const wstring &value) {
  dest.add_wstring(value);
}
