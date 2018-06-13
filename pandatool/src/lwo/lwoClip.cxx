/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoClip.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoClip.h"
#include "iffInputFile.h"
#include "lwoStillImage.h"

#include "indent.h"

TypeHandle LwoClip::_type_handle;

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoClip::
read_iff(IffInputFile *in, size_t stop_at) {
  _index = in->get_be_int32();
  read_subchunks_iff(in, stop_at);
  return true;
}

/**
 *
 */
void LwoClip::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  indent(out, indent_level + 2)
    << "index = " << _index << "\n";
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

/**
 * Allocates and returns a new chunk of the appropriate type based on the
 * given ID, according to the context given by this chunk itself.
 */
IffChunk *LwoClip::
make_new_chunk(IffInputFile *in, IffId id) {
  if (id == IffId("STIL")) {
    return new LwoStillImage;

  } else {
    return IffChunk::make_new_chunk(in, id);
  }
}
