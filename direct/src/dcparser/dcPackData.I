/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPackData.I
 * @author drose
 * @date 2004-06-15
 */

/**
 *
 */
INLINE DCPackData::
DCPackData() {
  _buffer = nullptr;
  _allocated_size = 0;
  _used_length = 0;
}

/**
 *
 */
INLINE DCPackData::
~DCPackData() {
  if (_buffer != nullptr) {
    delete[] _buffer;
  }
}

/**
 * Empties the contents of the data (without necessarily freeing its allocated
 * memory).
 */
INLINE void DCPackData::
clear() {
  _used_length = 0;
}

/**
 * Adds the indicated bytes to the end of the data.
 */
INLINE void DCPackData::
append_data(const char *buffer, size_t size) {
  set_used_length(_used_length + size);
  memcpy(_buffer + _used_length - size, buffer, size);
}

/**
 * Adds the indicated number of bytes to the end of the data without
 * initializing them, and returns a pointer to the beginning of the new data.
 */
INLINE char *DCPackData::
get_write_pointer(size_t size) {
  set_used_length(_used_length + size);
  return _buffer + _used_length - size;
}

/**
 * Adds some uninitialized bytes to the end of the data.
 */
INLINE void DCPackData::
append_junk(size_t size) {
  set_used_length(_used_length + size);
}

/**
 * Changes the data at the indicated position to the given value.  It is an
 * error if there are not at least position + size bytes in the data.
 */
INLINE void DCPackData::
rewrite_data(size_t position, const char *buffer, size_t size) {
  nassertv(position + size <= _used_length);
  memcpy(_buffer + position, buffer, size);
}

/**
 * Returns a pointer into the middle of the data at the indicated point.
 */
INLINE char *DCPackData::
get_rewrite_pointer(size_t position, size_t size) {
  nassertr(position + size <= _used_length, nullptr);
  return _buffer + position;
}

/**
 * Returns the data buffer as a string.  Also see get_data().
 */
INLINE std::string DCPackData::
get_string() const {
  return std::string(_buffer, _used_length);
}

/**
 * Returns the current length of the buffer.  This is the number of useful
 * bytes stored in the buffer, not the amount of memory it takes up.
 */
INLINE size_t DCPackData::
get_length() const {
  return _used_length;
}

/**
 * Returns the beginning of the data buffer.  The buffer is not null-
 * terminated, but see also get_string(). This may (or may not) return NULL if
 * the buffer is empty.
 *
 * This may be used in conjunction with get_length() to copy all of the bytes
 * out of the buffer.
 */
INLINE const char *DCPackData::
get_data() const {
  return _buffer;
}

/**
 * Returns the pointer to the beginning of the data buffer, and transfers
 * ownership of the buffer to the caller.  The caller is now responsible for
 * ultimately freeing the returned pointer with delete[], if it is non-NULL.
 * This may (or may not) return NULL if the buffer is empty.
 *
 * This also empties the DCPackData structure, and sets its length to zero (so
 * you should call get_length() before calling this method).
 */
INLINE char *DCPackData::
take_data() {
  char *data = _buffer;

  _buffer = nullptr;
  _allocated_size = 0;
  _used_length = 0;

  return data;
}
