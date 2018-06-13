/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceBlockTMap.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoSurfaceBlockTMap.h"
#include "lwoInputFile.h"
#include "lwoSurfaceBlockCoordSys.h"
#include "lwoSurfaceBlockTransform.h"
#include "lwoSurfaceBlockRefObj.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoSurfaceBlockTMap::_type_handle;

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoSurfaceBlockTMap::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  read_subchunks_iff(lin, stop_at);

  return true;
}

/**
 *
 */
void LwoSurfaceBlockTMap::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

/**
 * Allocates and returns a new chunk of the appropriate type based on the
 * given ID, according to the context given by this chunk itself.
 */
IffChunk *LwoSurfaceBlockTMap::
make_new_chunk(IffInputFile *in, IffId id) {
  if (id == IffId("CNTR") ||
      id == IffId("SIZE") ||
      id == IffId("ROTA")) {
    return new LwoSurfaceBlockTransform;

  } else if (id == IffId("OREF")) {
    return new LwoSurfaceBlockRefObj;

  } else if (id == IffId("CSYS")) {
    return new LwoSurfaceBlockCoordSys;

  } else {
    return IffChunk::make_new_chunk(in, id);
  }
}
