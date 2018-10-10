/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagram.I
 * @author drose
 * @date 2000-06-06
 */

/**
 * Constructs a datagram from an existing block of data.
 */
INLINE Datagram::
Datagram(const void *data, size_t size) {
  append_data(data, size);
}

/**
 * Constructs a datagram from an existing block of data.
 */
INLINE Datagram::
Datagram(vector_uchar data) :
  _data(std::move(data)) {
}

/**
 * Adds a boolean value to the datagram.
 */
INLINE void Datagram::
add_bool(bool b) {
  add_uint8(b);
}

/**
 * Adds a signed 8-bit integer to the datagram.
 */
INLINE void Datagram::
add_int8(int8_t value) {
  append_data(&value, 1);
}

/**
 * Adds an unsigned 8-bit integer to the datagram.
 */
INLINE void Datagram::
add_uint8(uint8_t value) {
  append_data(&value, 1);
}

/**
 * Adds a signed 16-bit integer to the datagram.
 */
INLINE void Datagram::
add_int16(int16_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 32-bit integer to the datagram.
 */
INLINE void Datagram::
add_int32(int32_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 64-bit integer to the datagram.
 */
INLINE void Datagram::
add_int64(int64_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 16-bit integer to the datagram.
 */
INLINE void Datagram::
add_uint16(uint16_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 32-bit integer to the datagram.
 */
INLINE void Datagram::
add_uint32(uint32_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 64-bit integer to the datagram.
 */
INLINE void Datagram::
add_uint64(uint64_t value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a 32-bit single-precision floating-point number to the datagram.
 * Since this kind of float is not necessarily portable across different
 * architectures, special care is required.
 */
INLINE void Datagram::
add_float32(PN_float32 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a 64-bit floating-point number to the datagram.
 */
INLINE void Datagram::
add_float64(PN_float64 value) {
  LittleEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds either a 32-bit or a 64-bit floating-point number, according to
 * set_stdfloat_double().
 */
INLINE void Datagram::
add_stdfloat(PN_stdfloat value) {
  if (_stdfloat_double) {
    add_float64((double)value);
  } else {
    add_float32((float)value);
  }
}

/**
 * Adds a signed 16-bit big-endian integer to the datagram.
 */
INLINE void Datagram::
add_be_int16(int16_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 32-bit big-endian integer to the datagram.
 */
INLINE void Datagram::
add_be_int32(int32_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a signed 64-bit big-endian integer to the datagram.
 */
INLINE void Datagram::
add_be_int64(int64_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 16-bit big-endian integer to the datagram.
 */
INLINE void Datagram::
add_be_uint16(uint16_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 32-bit big-endian integer to the datagram.
 */
INLINE void Datagram::
add_be_uint32(uint32_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds an unsigned 64-bit big-endian integer to the datagram.
 */
INLINE void Datagram::
add_be_uint64(uint64_t value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a 32-bit single-precision big-endian floating-point number to the
 * datagram.
 */
INLINE void Datagram::
add_be_float32(PN_float32 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a 64-bit big-endian floating-point number to the datagram.
 */
INLINE void Datagram::
add_be_float64(PN_float64 value) {
  BigEndian s(&value, sizeof(value));
  append_data(s.get_data(), sizeof(value));
}

/**
 * Adds a variable-length string to the datagram.  This actually adds a count
 * followed by n bytes.
 */
INLINE void Datagram::
add_string(const std::string &str) {
  // The max sendable length for a string is 2^16.
  nassertv(str.length() <= (uint16_t)0xffff);

  // Strings always are preceded by their length
  add_uint16((uint16_t)str.length());

  // Add the string
  append_data(str.data(), str.length());
}

/**
 * Adds a variable-length string to the datagram, using a 32-bit length field
 * to allow very long strings.
 */
INLINE void Datagram::
add_string32(const std::string &str) {
  // Strings always are preceded by their length
  add_uint32((uint32_t)str.length());

  // Add the string
  append_data(str.data(), str.length());
}

/**
 * Adds a variable-length string to the datagram, as a NULL-terminated string.
 */
INLINE void Datagram::
add_z_string(const std::string &str) {
  // We must not have any nested null characters in the string.
  size_t null_pos = str.find('\0');
  // Add the string (sans the null character).
  append_data(str.data(), std::min<size_t>(null_pos, str.length()));

  // And the null character.
  add_uint8('\0');
}

/**
 * Adds a fixed-length string to the datagram.  If the string given is less
 * than the requested size, this will pad the string out with zeroes; if it is
 * greater than the requested size, this will silently truncate the string.
 */
INLINE void Datagram::
add_fixed_string(const std::string &str, size_t size) {
  if (str.length() < size) {
    append_data(str.data(), str.length());
    pad_bytes(size - str.length());

  } else { // str.length() >= size
    append_data(str.data(), size);
  }
}

/**
 * Adds a variable-length binary blob to the datagram.  This actually adds a
 * count followed by n bytes.
 */
INLINE void Datagram::
add_blob(const vector_uchar &data) {
  // The max sendable size for a blob is 2^16.
  nassertv(data.size() <= (uint16_t)0xffff);

  // Blobs always are preceded by their size
  add_uint16((uint16_t)data.size());

  // Add the blob
  append_data(data.data(), data.size());
}

/**
 * Adds a variable-length binary blob to the datagram, using a 32-bit length
 * field to allow very long blobs.
 */
INLINE void Datagram::
add_blob32(const vector_uchar &data) {
  // Blobs always are preceded by their size
  add_uint32((uint32_t)data.size());

  // Add the blob
  append_data(data.data(), data.size());
}

/**
 * Appends some more raw data to the end of the datagram.
 */
INLINE void Datagram::
append_data(const vector_uchar &data) {
  append_data(data.data(), data.size());
}

/**
 * Returns the datagram's data as a string.
 */
INLINE std::string Datagram::
get_message() const {
  // Silly special case for gcc 3.2, which can't tolerate string(NULL, 0).
  if (_data.size() == 0) {
    return std::string();
  } else {
    return std::string((const char *)_data.p(), _data.size());
  }
}

/**
 * Returns a pointer to the beginning of the datagram's data.
 */
INLINE const void *Datagram::
get_data() const {
  return _data.p();
}

/**
 * Returns the number of bytes in the datagram.
 */
INLINE size_t Datagram::
get_length() const {
  return _data.size();
}

/**
 * Replaces the data in the Datagram with the data in the indicated PTA_uchar.
 * This is assignment by reference: subsequent changes to the Datagram will
 * also change the source PTA_uchar.
 */
INLINE void Datagram::
set_array(PTA_uchar data) {
  _data = data;
}

/**
 * Replaces the data in the Datagram with a copy of the data in the indicated
 * CPTA_uchar.  Unlike set_array(), a complete copy is made of the data;
 * subsequent changes to the Datagram will *not* change the source CPTA_uchar.
 */
INLINE void Datagram::
copy_array(CPTA_uchar data) {
  _data.clear();
  _data.v() = data.v();
}

/**
 * Returns a const pointer to the actual data in the Datagram.
 */
INLINE CPTA_uchar Datagram::
get_array() const {
  return _data;
}

/**
 * Returns a modifiable pointer to the actual data in the Datagram.
 */
INLINE PTA_uchar Datagram::
modify_array() {
  if (_data == nullptr) {
    // Create a new array.
    _data = PTA_uchar::empty_array(0);

  } else if (_data.get_ref_count() != 1) {
    // Copy on write.
    PTA_uchar new_data = PTA_uchar::empty_array(0);
    new_data.v() = _data.v();
    _data = new_data;
  }

  return _data;
}

/**
 * Changes the stdfloat_double flag, which defines the operation performed by
 * add_stdfloat() and DatagramIterator::get_stdfloat().  When this is true,
 * add_stdfloat() adds a 64-bit floating-point number; when it is false, it
 * adds a 32-bit floating-point number.  The default is based on the
 * STDFLOAT_DOUBLE compilation flag.
 */
INLINE void Datagram::
set_stdfloat_double(bool stdfloat_double) {
  _stdfloat_double = stdfloat_double;
}

/**
 * Returns the stdfloat_double flag.  See set_stdfloat_double().
 */
INLINE bool Datagram::
get_stdfloat_double() const {
  return _stdfloat_double;
}

/**
 *
 */
INLINE bool Datagram::
operator == (const Datagram &other) const {
  if (_data == other._data) {
    return true;
  }
  if (_data != nullptr && other._data != nullptr) {
    return _data.v() == other._data.v();
  }
  return false;
}

/**
 *
 */
INLINE bool Datagram::
operator != (const Datagram &other) const {
  return !operator == (other);
}

/**
 *
 */
INLINE bool Datagram::
operator < (const Datagram &other) const {
  if (_data == other._data) {
    // Same pointers.
    return false;
  }

  if (_data != nullptr && other._data != nullptr) {
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
generic_write_datagram(Datagram &dest, const std::string &value) {
  dest.add_string(value);
}

INLINE void
generic_write_datagram(Datagram &dest, const std::wstring &value) {
  dest.add_wstring(value);
}

INLINE void
generic_write_datagram(Datagram &dest, const vector_uchar &value) {
  dest.add_blob(value);
}
