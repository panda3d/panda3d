/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iffChunk.cxx
 * @author drose
 * @date 2001-04-23
 */

#include "iffChunk.h"
#include "iffInputFile.h"

#include "indent.h"

TypeHandle IffChunk::_type_handle;

/**
 *
 */
void IffChunk::
output(std::ostream &out) const {
  out << _id << " (" << get_type() << ")";
}

/**
 *
 */
void IffChunk::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << _id << " { ... }\n";
}

/**
 * Allocates and returns a new chunk of the appropriate type based on the
 * given ID, according to the context given by this chunk itself.
 */
IffChunk *IffChunk::
make_new_chunk(IffInputFile *in, IffId id) {
  return in->make_new_chunk(id);
}
