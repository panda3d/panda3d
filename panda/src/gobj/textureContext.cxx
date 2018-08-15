/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureContext.cxx
 * @author drose
 * @date 1999-10-07
 */

#include "textureContext.h"

TypeHandle TextureContext::_type_handle;

/**
 * Returns an implementation-defined handle or pointer that can be used
 * to interface directly with the underlying API.
 * Returns 0 if the underlying implementation does not support this.
 */
uint64_t TextureContext::
get_native_id() const {
  return 0;
}

/**
 * Similar to get_native_id, but some implementations use a separate
 * identifier for the buffer object associated with buffer textures.
 * Returns 0 if the underlying implementation does not support this, or
 * if this is not a buffer texture.
 */
uint64_t TextureContext::
get_native_buffer_id() const {
  return 0;
}

/**
 *
 */
void TextureContext::
output(std::ostream &out) const {
  out << *get_texture() << ", " << get_data_size_bytes();
}

/**
 *
 */
void TextureContext::
write(std::ostream &out, int indent_level) const {
  SavedContext::write(out, indent_level);
}
