// Filename: lwoSurface.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoSurface.h"
#include "iffInputFile.h"
#include "lwoSurfaceColor.h"
#include "lwoSurfaceParameter.h"
#include "lwoSurfaceSidedness.h"

#include <indent.h>

TypeHandle LwoSurface::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoSurface::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoSurface::
read_iff(IffInputFile *in, size_t stop_at) {
  _name = in->get_string();
  _source = in->get_string();
  return read_subchunks_iff(in, stop_at);
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurface::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoSurface::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  indent(out, indent_level + 2)
    << "name = \"" << _name << "\", source = \"" << _source << "\"\n";
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurface::make_new_chunk
//       Access: Protected, Virtual
//  Description: Allocates and returns a new chunk of the appropriate
//               type based on the given ID, according to the context
//               given by this chunk itself.
////////////////////////////////////////////////////////////////////
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
  
  } else {
    return IffChunk::make_new_chunk(in, id);
  }
}

