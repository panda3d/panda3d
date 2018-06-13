/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexBufferContext.cxx
 * @author drose
 * @date 2005-03-17
 */

#include "vertexBufferContext.h"
#include "config_gobj.h"

TypeHandle VertexBufferContext::_type_handle;

/**
 *
 */
void VertexBufferContext::
output(std::ostream &out) const {
  out << *get_data() << ", " << get_data_size_bytes();
}

/**
 *
 */
void VertexBufferContext::
write(std::ostream &out, int indent_level) const {
  SavedContext::write(out, indent_level);
}
