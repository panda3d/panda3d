// Filename: lwoSurfaceBlockHeader.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoSurfaceBlockHeader.h"
#include "lwoInputFile.h"

#include <indent.h>

TypeHandle LwoSurfaceBlockHeader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockHeader::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoSurfaceBlockHeader::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _ordinal = lin->get_string();
  read_subchunks_iff(lin, stop_at);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockHeader::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoSurfaceBlockHeader::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  indent(out, indent_level + 2)
    << "ordinal = 0x" << hex << setfill('0');

  string::const_iterator si;
  for (si = _ordinal.begin(); si != _ordinal.end(); ++si) {
    out << setw(2) << (int)(unsigned char)(*si);
  }

  out << dec << setfill(' ') << "\n";
  
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockHeader::make_new_chunk
//       Access: Protected, Virtual
//  Description: Allocates and returns a new chunk of the appropriate
//               type based on the given ID, according to the context
//               given by this chunk itself.
////////////////////////////////////////////////////////////////////
IffChunk *LwoSurfaceBlockHeader::
make_new_chunk(IffInputFile *in, IffId id) {
  /*
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
  
    } else */ {
    return IffChunk::make_new_chunk(in, id);
  }
}

