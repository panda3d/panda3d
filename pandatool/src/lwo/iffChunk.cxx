// Filename: iffChunk.cxx
// Created by:  drose (23Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "iffChunk.h"
#include "iffInputFile.h"

#include <indent.h>

TypeHandle IffChunk::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: IffChunk::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void IffChunk::
output(ostream &out) const {
  out << _id << " (" << get_type() << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: IffChunk::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void IffChunk::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _id << " { ... }\n";
}

////////////////////////////////////////////////////////////////////
//     Function: IffChunk::make_new_chunk
//       Access: Public, Virtual
//  Description: Allocates and returns a new chunk of the appropriate
//               type based on the given ID, according to the context
//               given by this chunk itself.
////////////////////////////////////////////////////////////////////
IffChunk *IffChunk::
make_new_chunk(IffInputFile *in, IffId id) {
  return in->make_new_chunk(id);
}
