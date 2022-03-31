/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramIterator.I
 * @author drose
 * @date 2001-05-08
 */

/**
 *
 */
INLINE DatagramIterator::
DatagramIterator() :
    _datagram(nullptr),
    _current_index(0) {
}

/**
 *
 */
INLINE DatagramIterator::
DatagramIterator(const Datagram &datagram, size_t offset) :
    _datagram(&datagram),
    _current_index(offset) {
  nassertv(_current_index <= _datagram->get_length());
}

/**
 * direct Assignment to a Datagram
 */
INLINE void DatagramIterator::
assign(Datagram &datagram, size_t offset) {
  _datagram = &datagram;
  _current_index = offset;
}

// Various ways to get data and increment the iterator... Cut-and-paste-orama

/**
 * Extracts a boolean value.
 */
INLINE bool DatagramIterator::
get_bool() {
  return get_uint8() != 0;
}

/**
 * Extracts a signed 8-bit integer.
 */
INLINE int8_t DatagramIterator::
get_int8() {
  nassertr(_datagram != nullptr, 0);
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index < _datagram->get_length(), 0);
  // Get the Data:
  const char *ptr = (const char *)_datagram->get_data();
  int8_t tempvar = (int8_t)ptr[_current_index];
  ++_current_index;

  return tempvar;
}

/**
 * Extracts an unsigned 8-bit integer.
 */
INLINE uint8_t DatagramIterator::
get_uint8() {
  nassertr(_datagram != nullptr, 0);
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index < _datagram->get_length(), 0);
  // Get the Data:
  const char *ptr = (const char *)_datagram->get_data();
  uint8_t tempvar = (uint8_t)ptr[_current_index];
  ++_current_index;

  return tempvar;
}

/**
 * Extracts a signed 16-bit integer.
 */
INLINE int16_t DatagramIterator::
get_int16() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  int16_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a signed 32-bit integer.
 */
INLINE int32_t DatagramIterator::
get_int32() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  int32_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a signed 64-bit integer.
 */
INLINE int64_t DatagramIterator::
get_int64() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  int64_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts an unsigned 16-bit integer.
 */
INLINE uint16_t DatagramIterator::
get_uint16() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  uint16_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts an unsigned 32-bit integer.
 */
INLINE uint32_t DatagramIterator::
get_uint32() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  uint32_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts an unsigned 64-bit integer.
 */
INLINE uint64_t DatagramIterator::
get_uint64() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  uint64_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a 32-bit single-precision floating-point number.
 */
INLINE PN_float32 DatagramIterator::
get_float32() {
  nassertr(_datagram != nullptr, 0.0);
  nassertr(_current_index < _datagram->get_length(), 0.0);

  PN_float32 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0.0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a 64-bit floating-point number.
 */
INLINE PN_float64 DatagramIterator::
get_float64() {
  nassertr(_datagram != nullptr, 0.0);
  nassertr(_current_index < _datagram->get_length(), 0.0);

  PN_float64 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0.0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}


/**
 * Extracts either a 32-bit or a 64-bit floating-point number, according to
 * Datagram::set_stdfloat_double().
 */
INLINE PN_stdfloat DatagramIterator::
get_stdfloat() {
  nassertr(_datagram != nullptr, 0.0);
  if (_datagram->get_stdfloat_double()) {
    return (PN_stdfloat)get_float64();
  } else {
    return (PN_stdfloat)get_float32();
  }
}

/**
 * Extracts a signed 16-bit big-endian integer.
 */
INLINE int16_t DatagramIterator::
get_be_int16() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  int16_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a signed 32-bit big-endian integer.
 */
INLINE int32_t DatagramIterator::
get_be_int32() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  int32_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a signed 64-bit big-endian integer.
 */
INLINE int64_t DatagramIterator::
get_be_int64() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  int64_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts an unsigned 16-bit big-endian integer.
 */
INLINE uint16_t DatagramIterator::
get_be_uint16() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  uint16_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts an unsigned 32-bit big-endian integer.
 */
INLINE uint32_t DatagramIterator::
get_be_uint32() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  uint32_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts an unsigned 64-bit big-endian integer.
 */
INLINE uint64_t DatagramIterator::
get_be_uint64() {
  nassertr(_datagram != nullptr, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  uint64_t tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a 32-bit big-endian single-precision floating-point number.
 */
INLINE PN_float32 DatagramIterator::
get_be_float32() {
  nassertr(_datagram != nullptr, 0.0);
  nassertr(_current_index < _datagram->get_length(), 0.0);

  PN_float32 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a 64-bit big-endian floating-point number.
 */
INLINE PN_float64 DatagramIterator::
get_be_float64() {
  nassertr(_datagram != nullptr, 0.0);
  nassertr(_current_index < _datagram->get_length(), 0.0);

  PN_float64 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0.0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

/**
 * Extracts a variable-length binary blob.
 */
INLINE vector_uchar DatagramIterator::
get_blob() {
  return extract_bytes(get_uint16());
}

/**
 * Extracts a variable-length binary blob with a 32-bit size field.
 */
INLINE vector_uchar DatagramIterator::
get_blob32() {
  return extract_bytes(get_uint32());
}

/**
 * Skips over the indicated number of bytes in the datagram.
 */
INLINE void DatagramIterator::
skip_bytes(size_t size) {
  nassertv(_datagram != nullptr);
  nassertv((int)size >= 0);
#ifndef NDEBUG
  if (_current_index + size > _datagram->get_length()) {
     nout << "datagram overflow: current_index = " << _current_index
          << " size = " << size << " length = " << _datagram->get_length() << "\n";
    _datagram->dump_hex(nout);
  }
#endif
  nassertv(_current_index + size <= _datagram->get_length());
  _current_index += size;
}

/**
 * Returns the remaining bytes in the datagram as a string, but does not
 * extract them from the iterator.
 */
INLINE vector_uchar DatagramIterator::
get_remaining_bytes() const {
  nassertr(_datagram != nullptr, vector_uchar());
  nassertr(_current_index <= _datagram->get_length(), vector_uchar());

  const unsigned char *ptr = (const unsigned char *)_datagram->get_data();
  return vector_uchar(ptr + _current_index, ptr + _datagram->get_length());
}

/**
 * Return the bytes left in the datagram.
 */
INLINE size_t DatagramIterator::
get_remaining_size() const {
  return _datagram->get_length() - _current_index;
}

/**
 * Return the datagram of this iterator.
 */
INLINE const Datagram &DatagramIterator::
get_datagram() const {
  return *_datagram;
}

/**
 * Returns the current position within the datagram of the next piece of data
 * to extract.
 */
INLINE size_t DatagramIterator::
get_current_index() const {
  return _current_index;
}

INLINE void
generic_read_datagram(bool &result, DatagramIterator &source) {
  result = source.get_bool();
}

INLINE void
generic_read_datagram(int &result, DatagramIterator &source) {
  result = source.get_int32();
}

INLINE void
generic_read_datagram(float &result, DatagramIterator &source) {
  result = source.get_float32();
}

INLINE void
generic_read_datagram(double &result, DatagramIterator &source) {
  result = source.get_float64();
}

INLINE void
generic_read_datagram(std::string &result, DatagramIterator &source) {
  result = source.get_string();
}

INLINE void
generic_read_datagram(std::wstring &result, DatagramIterator &source) {
  result = source.get_wstring();
}

INLINE void
generic_read_datagram(vector_uchar &result, DatagramIterator &source) {
  result = source.get_blob();
}
