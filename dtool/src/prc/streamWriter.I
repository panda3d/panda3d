/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamWriter.I
 * @author drose
 * @date 2002-08-04
 */

/**
 *
 */
INLINE StreamWriter::
StreamWriter(std::ostream &out) :
#ifdef HAVE_PYTHON
  softspace(0),
#endif
  _out(&out),
  _owns_stream(false)
{
}

/**
 *
 */
INLINE StreamWriter::
StreamWriter(std::ostream *out, bool owns_stream) :
#ifdef HAVE_PYTHON
  softspace(0),
#endif
  _out(out),
  _owns_stream(owns_stream)
{
}

/**
 * The copy constructor does not copy ownership of the stream.
 */
INLINE StreamWriter::
StreamWriter(const StreamWriter &copy) :
#ifdef HAVE_PYTHON
  softspace(0),
#endif
  _out(copy._out),
  _owns_stream(false)
{
}

/**
 * The move constructor steals ownership of the stream.
 */
INLINE StreamWriter::
StreamWriter(StreamWriter &&from) noexcept :
#ifdef HAVE_PYTHON
  softspace(0),
#endif
  _out(from._out),
  _owns_stream(from._owns_stream)
{
  from._owns_stream = false;
}

/**
 * The copy assignment operator does not copy ownership of the stream.
 */
INLINE void StreamWriter::
operator = (const StreamWriter &copy) {
  if (_owns_stream) {
    delete _out;
  }
  _out = copy._out;
  _owns_stream = false;
}

/**
 * The move assignment operator steals ownership of the stream.
 */
INLINE void StreamWriter::
operator = (StreamWriter &&from) noexcept {
  if (_owns_stream) {
    delete _out;
  }
  _out = from._out;
  _owns_stream = from._owns_stream;
  from._owns_stream = false;
}

/**
 *
 */
INLINE StreamWriter::
~StreamWriter() {
  if (_owns_stream) {
    delete _out;
  }
}

/**
 * Returns the stream in use.
 */
INLINE std::ostream *StreamWriter::
get_ostream() const {
  return _out;
}

/**
 * Adds a boolean value to the stream.
 */
INLINE void StreamWriter::
add_bool(bool b) {
  add_uint8(b);
}

/**
 * Adds a signed 8-bit integer to the stream.
 */
INLINE void StreamWriter::
add_int8(int8_t value) {
  append_data(&value, 1);
}

/**
 * Adds an unsigned 8-bit integer to the stream.
 */
INLINE void StreamWriter::
add_uint8(uint8_t value) {
  append_data(&value, 1);
}

/**
 * Adds a signed 16-bit integer to the stream.
 */
INLINE void StreamWriter::
add_int16(int16_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 32-bit integer to the stream.
 */
INLINE void StreamWriter::
add_int32(int32_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 64-bit integer to the stream.
 */
INLINE void StreamWriter::
add_int64(int64_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 16-bit integer to the stream.
 */
INLINE void StreamWriter::
add_uint16(uint16_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 32-bit integer to the stream.
 */
INLINE void StreamWriter::
add_uint32(uint32_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 64-bit integer to the stream.
 */
INLINE void StreamWriter::
add_uint64(uint64_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a 32-bit single-precision floating-point number to the stream.  Since
 * this kind of float is not necessarily portable across different
 * architectures, special care is required.
 */
INLINE void StreamWriter::
add_float32(float value) {
  // For now, we assume the float format is portable across all architectures
  // we are concerned with.  If we come across one that is different, we will
  // have to convert.
  nassertv(sizeof(value) == 4);
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a 64-bit floating-point number to the stream.
 */
INLINE void StreamWriter::
add_float64(PN_float64 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 16-bit big-endian integer to the streamWriter.
 */
INLINE void StreamWriter::
add_be_int16(int16_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 32-bit big-endian integer to the streamWriter.
 */
INLINE void StreamWriter::
add_be_int32(int32_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 64-bit big-endian integer to the streamWriter.
 */
INLINE void StreamWriter::
add_be_int64(int64_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 16-bit big-endian integer to the streamWriter.
 */
INLINE void StreamWriter::
add_be_uint16(uint16_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 32-bit big-endian integer to the streamWriter.
 */
INLINE void StreamWriter::
add_be_uint32(uint32_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 64-bit big-endian integer to the streamWriter.
 */
INLINE void StreamWriter::
add_be_uint64(uint64_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a 32-bit single-precision big-endian floating-point number to the
 * stream.  Since this kind of float is not necessarily portable across
 * different architectures, special care is required.
 */
INLINE void StreamWriter::
add_be_float32(float value) {
  // For now, we assume the float format is portable across all architectures
  // we are concerned with.  If we come across one that is different, we will
  // have to convert.
  nassertv(sizeof(value) == 4);
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a 64-bit big-endian floating-point number to the streamWriter.
 */
INLINE void StreamWriter::
add_be_float64(PN_float64 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a variable-length string to the stream.  This actually adds a count
 * followed by n bytes.
 */
INLINE void StreamWriter::
add_string(const std::string &str) {
  // The max sendable length for a string is 2^16.
  nassertv(str.length() <= (uint16_t)0xffff);

  // Strings always are preceded by their length
  add_uint16((uint16_t)str.length());

  // Add the string
  append_data(str);
}

/**
 * Adds a variable-length string to the stream, using a 32-bit length field.
 */
INLINE void StreamWriter::
add_string32(const std::string &str) {
  // Strings always are preceded by their length
  add_uint32((uint32_t)str.length());

  // Add the string
  append_data(str);
}

/**
 * Adds a variable-length string to the stream, as a NULL-terminated string.
 */
INLINE void StreamWriter::
add_z_string(std::string str) {
  // We must not have any nested null characters in the string.
  size_t null_pos = str.find('\0');
  // Add the string (sans the null character).
  append_data(str.substr(0, null_pos));

  // And the null character.
  add_uint8('\0');
}

/**
 * Adds a fixed-length string to the stream.  If the string given is less than
 * the requested size, this will pad the string out with zeroes; if it is
 * greater than the requested size, this will silently truncate the string.
 */
INLINE void StreamWriter::
add_fixed_string(const std::string &str, size_t size) {
  if (str.length() < size) {
    append_data(str);
    pad_bytes(size - str.length());

  } else { // str.length() >= size
    append_data(str.substr(0, size));
  }
}

/**
 * Appends some more raw data to the end of the streamWriter.
 */
INLINE void StreamWriter::
append_data(const void *data, size_t size) {
  _out->write((const char *)data, size);
}

/**
 * Appends some more raw data to the end of the streamWriter.
 */
INLINE void StreamWriter::
append_data(const std::string &data) {
  append_data(data.data(), data.length());
}

/**
 * Calls flush() on the underlying stream.
 */
INLINE void StreamWriter::
flush() {
  _out->flush();
}

/**
 * A synonym of append_data().  This is useful when assigning the StreamWriter
 * to sys.stderr and/or sys.stdout in Python.
 */
INLINE void StreamWriter::
write(const std::string &data) {
  append_data(data.data(), data.length());
}
