/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBuffer.I
 * @author rdb
 * @date 2016-12-12
 */

/**
 * Creates an uninitialized buffer object with the given size.  For now, these
 * parameters cannot be modified, but this may change in the future.
 */
INLINE ShaderBuffer::
ShaderBuffer(const std::string &name, uint64_t size, UsageHint usage_hint) :
  Namable(name),
  _data_size_bytes(size),
  _usage_hint(usage_hint) {
}

/**
 * Creates a buffer object initialized with the given data.  For now, these
 * parameters cannot be modified, but this may change in the future.
 */
INLINE ShaderBuffer::
ShaderBuffer(const std::string &name, vector_uchar initial_data, UsageHint usage_hint) :
  Namable(name),
  _data_size_bytes(initial_data.size()),
  _usage_hint(usage_hint),
  _initial_data(std::move(initial_data)) {

  // Make sure it is padded to 16 bytes.  Some drivers like that.
  if ((_initial_data.size() & 15u) != 0) {
    _initial_data.resize((_initial_data.size() + 15u) & ~15u, 0);
    _data_size_bytes = _initial_data.size();
  }
}

/**
 * Returns the buffer size in bytes.
 */
INLINE uint64_t ShaderBuffer::
get_data_size_bytes() const {
  return _data_size_bytes;
}

/**
 * Returns the buffer usage hint.
 */
INLINE GeomEnums::UsageHint ShaderBuffer::
get_usage_hint() const {
  return _usage_hint;
}

/**
 * Returns a pointer to the initial buffer data, or NULL if not specified.
 */
INLINE const unsigned char *ShaderBuffer::
get_initial_data() const {
  if (_initial_data.empty()) {
    return nullptr;
  } else {
    return &_initial_data[0];
  }
}
