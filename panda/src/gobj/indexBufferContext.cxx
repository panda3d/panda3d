/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indexBufferContext.cxx
 * @author drose
 * @date 2005-03-17
 */

#include "indexBufferContext.h"

TypeHandle IndexBufferContext::_type_handle;

/**
 *
 */
void IndexBufferContext::
output(std::ostream &out) const {
  out << *get_data() << ", " << get_data_size_bytes();
}

/**
 *
 */
void IndexBufferContext::
write(std::ostream &out, int indent_level) const {
  SavedContext::write(out, indent_level);
}
