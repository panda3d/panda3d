// Filename: lwoSurfaceBlock.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoSurfaceBlock.h"
#include "iffInputFile.h"
#include "lwoSurfaceBlockHeader.h"
#include "lwoSurfaceBlockTMap.h"

#include <indent.h>

TypeHandle LwoSurfaceBlock::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlock::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoSurfaceBlock::
read_iff(IffInputFile *in, size_t stop_at) {
  read_subchunks_iff(in, stop_at);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlock::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoSurfaceBlock::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlock::make_new_chunk
//       Access: Protected, Virtual
//  Description: Allocates and returns a new chunk of the appropriate
//               type based on the given ID, according to the context
//               given by this chunk itself.
////////////////////////////////////////////////////////////////////
IffChunk *LwoSurfaceBlock::
make_new_chunk(IffInputFile *in, IffId id) {
  if (id == IffId("IMAP") ||
      id == IffId("PROC") ||
      id == IffId("GRAD") ||
      id == IffId("SHDR")) {
    return new LwoSurfaceBlockHeader;

  } else if (id == IffId("TMAP")) {
    return new LwoSurfaceBlockTMap;

  } else  {
    return IffChunk::make_new_chunk(in, id);
  }
}

