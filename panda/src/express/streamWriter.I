// Filename: streamWriter.I
// Created by:  drose (04Aug02)
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
//     Function: StreamWriter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StreamWriter::
StreamWriter(ostream &out) : 
  _out(&out)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StreamWriter::
StreamWriter(ostream *out) : 
  _out(out)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StreamWriter::
StreamWriter(const StreamWriter &copy) :
  _out(copy._out)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
operator = (const StreamWriter &copy) {
  _out = copy._out;
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StreamWriter::
~StreamWriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::get_ostream
//       Access: Public
//  Description: Returns the stream in use.
////////////////////////////////////////////////////////////////////
INLINE ostream *StreamWriter::
get_ostream() const {
  return _out;
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_bool
//       Access: Public
//  Description: Adds a boolean value to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_bool(bool b) {
  add_uint8(b);
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_int8
//       Access: Public
//  Description: Adds a signed 8-bit integer to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_int8(PN_int8 value) {
  append_data(&value, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_uint8
//       Access: Public
//  Description: Adds an unsigned 8-bit integer to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_uint8(PN_uint8 value) {
  append_data(&value, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_int16
//       Access: Public
//  Description: Adds a signed 16-bit integer to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_int16(PN_int16 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_int32
//       Access: Public
//  Description: Adds a signed 32-bit integer to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_int32(PN_int32 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_int64
//       Access: Public
//  Description: Adds a signed 64-bit integer to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_int64(PN_int64 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_uint16
//       Access: Public
//  Description: Adds an unsigned 16-bit integer to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_uint16(PN_uint16 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_uint32
//       Access: Public
//  Description: Adds an unsigned 32-bit integer to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_uint32(PN_uint32 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_uint64
//       Access: Public
//  Description: Adds an unsigned 64-bit integer to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_uint64(PN_uint64 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_float32
//       Access: Public
//  Description: Adds a 32-bit single-precision floating-point number
//               to the stream.  Since this kind of float is not
//               necessarily portable across different architectures,
//               special care is required.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_float32(float value) {
  // For now, we assume the float format is portable across all
  // architectures we are concerned with.  If we come across one that
  // is different, we will have to convert.
  nassertv(sizeof(value) == 4);
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_float64
//       Access: Public
//  Description: Adds a 64-bit floating-point number to the stream.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_float64(PN_float64 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_be_int16
//       Access: Public
//  Description: Adds a signed 16-bit big-endian integer to the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_be_int16(PN_int16 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_be_int32
//       Access: Public
//  Description: Adds a signed 32-bit big-endian integer to the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_be_int32(PN_int32 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_be_int64
//       Access: Public
//  Description: Adds a signed 64-bit big-endian integer to the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_be_int64(PN_int64 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_be_uint16
//       Access: Public
//  Description: Adds an unsigned 16-bit big-endian integer to the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_be_uint16(PN_uint16 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_be_uint32
//       Access: Public
//  Description: Adds an unsigned 32-bit big-endian integer to the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_be_uint32(PN_uint32 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_be_uint64
//       Access: Public
//  Description: Adds an unsigned 64-bit big-endian integer to the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_be_uint64(PN_uint64 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_be_float32
//       Access: Public
//  Description: Adds a 32-bit single-precision big-endian
//               floating-point number to the stream.  Since this
//               kind of float is not necessarily portable across
//               different architectures, special care is required.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_be_float32(float value) {
  // For now, we assume the float format is portable across all
  // architectures we are concerned with.  If we come across one that
  // is different, we will have to convert.
  nassertv(sizeof(value) == 4);
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_be_float64
//       Access: Public
//  Description: Adds a 64-bit big-endian floating-point number to the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_be_float64(PN_float64 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_string
//       Access: Public
//  Description: Adds a variable-length string to the stream.  This
//               actually adds a count followed by n bytes.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_string(const string &str) {
  // The max sendable length for a string is 2^16.
  nassertv(str.length() <= (PN_uint16)0xffff);

  // Strings always are preceded by their length
  add_uint16(str.length());

  // Add the string
  append_data(str);
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_string32
//       Access: Public
//  Description: Adds a variable-length string to the stream, using a
//               32-bit length field.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_string32(const string &str) {
  // Strings always are preceded by their length
  add_uint32(str.length());

  // Add the string
  append_data(str);
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_z_string
//       Access: Public
//  Description: Adds a variable-length string to the stream, as a
//               NULL-terminated string.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_z_string(string str) {
  // We must not have any nested null characters in the string.
  size_t null_pos = str.find('\0');
  // Add the string (sans the null character).
  append_data(str.substr(0, null_pos));

  // And the null character.
  add_uint8('\0');
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::add_fixed_string
//       Access: Public
//  Description: Adds a fixed-length string to the stream.  If the
//               string given is less than the requested size, this
//               will pad the string out with zeroes; if it is greater
//               than the requested size, this will silently truncate
//               the string.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
add_fixed_string(const string &str, size_t size) {
  if (str.length() < size) {
    append_data(str);
    pad_bytes(size - str.length());

  } else { // str.length() >= size
    append_data(str.substr(0, size));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::append_data
//       Access: Public
//  Description: Appends some more raw data to the end of the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
append_data(const void *data, size_t size) {
  _out->write((const char *)data, size);
}

////////////////////////////////////////////////////////////////////
//     Function: StreamWriter::append_data
//       Access: Public
//  Description: Appends some more raw data to the end of the
//               streamWriter.
////////////////////////////////////////////////////////////////////
INLINE void StreamWriter::
append_data(const string &data) {
  append_data(data.data(), data.length());
}
