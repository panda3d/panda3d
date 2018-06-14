/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceBlock.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoSurfaceBlock.h"
#include "iffInputFile.h"
#include "lwoSurfaceBlockAxis.h"
#include "lwoSurfaceBlockImage.h"
#include "lwoSurfaceBlockHeader.h"
#include "lwoSurfaceBlockProjection.h"
#include "lwoSurfaceBlockRepeat.h"
#include "lwoSurfaceBlockTMap.h"
#include "lwoSurfaceBlockWrap.h"
#include "lwoSurfaceBlockVMapName.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoSurfaceBlock::_type_handle;

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoSurfaceBlock::
read_iff(IffInputFile *in, size_t stop_at) {
  PT(IffChunk) chunk = in->get_subchunk(this);
  if (chunk == nullptr) {
    return false;
  }
  if (!chunk->is_of_type(LwoSurfaceBlockHeader::get_class_type())) {
    nout << "Invalid chunk for header of surface block: " << *chunk << "\n";
    return false;
  }

  _header = DCAST(LwoSurfaceBlockHeader, chunk);

  read_subchunks_iff(in, stop_at);
  return true;
}

/**
 *
 */
void LwoSurfaceBlock::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  _header->write(out, indent_level + 2);
  out << "\n";
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

/**
 * Allocates and returns a new chunk of the appropriate type based on the
 * given ID, according to the context given by this chunk itself.
 */
IffChunk *LwoSurfaceBlock::
make_new_chunk(IffInputFile *in, IffId id) {
  if (id == IffId("IMAP") ||
      id == IffId("PROC") ||
      id == IffId("GRAD") ||
      id == IffId("SHDR")) {
    return new LwoSurfaceBlockHeader;

  } else if (id == IffId("TMAP")) {
    return new LwoSurfaceBlockTMap;

  } else if (id == IffId("PROJ")) {
    return new LwoSurfaceBlockProjection;

  } else if (id == IffId("AXIS")) {
    return new LwoSurfaceBlockAxis;

  } else if (id == IffId("IMAG")) {
    return new LwoSurfaceBlockImage;

  } else if (id == IffId("WRAP")) {
    return new LwoSurfaceBlockWrap;

  } else if (id == IffId("WRPH") ||
             id == IffId("WRPW")) {
    return new LwoSurfaceBlockRepeat;

  } else if (id == IffId("VMAP")) {
    return new LwoSurfaceBlockVMapName;

  } else  {
    return IffChunk::make_new_chunk(in, id);
  }
}
