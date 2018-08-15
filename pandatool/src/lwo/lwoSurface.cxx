/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurface.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoSurface.h"
#include "iffInputFile.h"
#include "lwoSurfaceBlock.h"
#include "lwoSurfaceColor.h"
#include "lwoSurfaceParameter.h"
#include "lwoSurfaceSidedness.h"
#include "lwoSurfaceSmoothingAngle.h"

#include "indent.h"

TypeHandle LwoSurface::_type_handle;

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoSurface::
read_iff(IffInputFile *in, size_t stop_at) {
  _name = in->get_string();
  _source = in->get_string();
  read_subchunks_iff(in, stop_at);
  return true;
}

/**
 *
 */
void LwoSurface::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  indent(out, indent_level + 2)
    << "name = \"" << _name << "\", source = \"" << _source << "\"\n";
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

/**
 * Allocates and returns a new chunk of the appropriate type based on the
 * given ID, according to the context given by this chunk itself.
 */
IffChunk *LwoSurface::
make_new_chunk(IffInputFile *in, IffId id) {
  if (id == IffId("COLR")) {
    return new LwoSurfaceColor;

  } else if (id == IffId("DIFF") ||
             id == IffId("LUMI") ||
             id == IffId("SPEC") ||
             id == IffId("REFL") ||
             id == IffId("TRAN") ||
             id == IffId("TRNL") ||
             id == IffId("GLOS") ||
             id == IffId("SHRP") ||
             id == IffId("BUMP") ||
             id == IffId("RSAN") ||
             id == IffId("RIND")) {
    return new LwoSurfaceParameter;

  } else if (id == IffId("SIDE")) {
    return new LwoSurfaceSidedness;

  } else if (id == IffId("SMAN")) {
    return new LwoSurfaceSmoothingAngle;

  } else if (id == IffId("BLOK")) {
    return new LwoSurfaceBlock;

  } else {
    return IffChunk::make_new_chunk(in, id);
  }
}
