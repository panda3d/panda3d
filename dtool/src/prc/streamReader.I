/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamReader.I
 * @author drose
 * @date 2002-08-04
 */

/**
 *
 */
INLINE StreamReader::
StreamReader(istream &in) :
  _in(&in),
  _owns_stream(false)
{
}

/**
 * If owns_stream is true, the stream pointer will be deleted when the
 * StreamReader destructs.
 */
INLINE StreamReader::
StreamReader(istream *in, bool owns_stream) :
  _in(in),
  _owns_stream(owns_stream)
{
}

/**
 * The copy constructor does not copy ownership of the stream.
 */
INLINE StreamReader::
StreamReader(const StreamReader &copy) :
  _in(copy._in),
  _owns_stream(false)
{
}

/**
 * The copy constructor does not copy ownership of the stream.
 */
INLINE void StreamReader::
operator = (const StreamReader &copy) {
  if (_owns_stream) {
    delete _in;
  }
  _in = copy._in;
  _owns_stream = false;
}

/**
 *
 */
INLINE StreamReader::
~StreamReader() {
  if (_owns_stream) {
    delete _in;
  }
}

/**
 * Returns the stream in use.
 */
INLINE istream *StreamReader::
get_istream() const {
  return _in;
}

/**
 * Extracts a boolean value.
 */
INLINE bool StreamReader::
get_bool() {
  return (get_uint8() != 0);
}

/**
 * Extracts a signed 8-bit integer.
 */
INLINE PN_int8 StreamReader::
get_int8() {
  return (PN_int8)_in->get();
}

/**
 * Extracts an unsigned 8-bit integer.
 */
INLINE PN_uint8 StreamReader::
get_uint8() {
  return (PN_uint8)_in->get();
}

/**
 * Extracts a signed 16-bit integer.
 */
INLINE PN_int16 StreamReader::
get_int16() {
  PN_int16 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  LittleEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a signed 32-bit integer.
 */
INLINE PN_int32 StreamReader::
get_int32() {
  PN_int32 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  LittleEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a signed 64-bit integer.
 */
INLINE PN_int64 StreamReader::
get_int64() {
  PN_int64 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  LittleEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts an unsigned 16-bit integer.
 */
INLINE PN_uint16 StreamReader::
get_uint16() {
  PN_uint16 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  LittleEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts an unsigned 32-bit integer.
 */
INLINE PN_uint32 StreamReader::
get_uint32() {
  PN_uint32 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  LittleEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts an unsigned 64-bit integer.
 */
INLINE PN_uint64 StreamReader::
get_uint64() {
  PN_uint64 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  LittleEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a 32-bit single-precision floating-point number.  Since this kind
 * of float is not necessarily portable across different architectures,
 * special care is required.
 */
INLINE float StreamReader::
get_float32() {
  // For now, we assume the float format is portable across all architectures
  // we are concerned with.  If we come across one that is different, we will
  // have to convert.
  nassertr(sizeof(float) == 4, 0.0f);

  float readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  LittleEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a 64-bit floating-point number.
 */
INLINE PN_float64 StreamReader::
get_float64() {
  PN_float64 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  LittleEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a signed big-endian 16-bit integer.
 */
INLINE PN_int16 StreamReader::
get_be_int16() {
  PN_int16 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  BigEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a signed big-endian 32-bit integer.
 */
INLINE PN_int32 StreamReader::
get_be_int32() {
  PN_int32 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  BigEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a signed big-endian 64-bit integer.
 */
INLINE PN_int64 StreamReader::
get_be_int64() {
  PN_int64 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  BigEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts an unsigned big-endian 16-bit integer.
 */
INLINE PN_uint16 StreamReader::
get_be_uint16() {
  PN_uint16 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  BigEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts an unsigned big-endian 32-bit integer.
 */
INLINE PN_uint32 StreamReader::
get_be_uint32() {
  PN_uint32 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  BigEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts an unsigned big-endian 64-bit integer.
 */
INLINE PN_uint64 StreamReader::
get_be_uint64() {
  PN_uint64 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  BigEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a 32-bit single-precision big-endian floating-point number.  Since
 * this kind of float is not necessarily portable across different
 * architectures, special care is required.
 */
INLINE float StreamReader::
get_be_float32() {
  // For now, we assume the float format is portable across all architectures
  // we are concerned with.  If we come across one that is different, we will
  // have to convert.
  nassertr(sizeof(float) == 4, 0.0f);

  float readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  BigEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}

/**
 * Extracts a 64-bit big-endian floating-point number.
 */
INLINE PN_float64 StreamReader::
get_be_float64() {
  PN_float64 readval, retval;
  _in->read((char *)&readval, sizeof(readval));
  BigEndian s(&readval, 0, sizeof(readval));
  s.store_value(&retval, sizeof(retval));
  return retval;
}
