// Filename: lwoHeader.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoHeader.h"
#include "iffInputFile.h"

#include <indent.h>

TypeHandle LwoHeader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoHeader::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoHeader::
read_iff(IffInputFile *in, size_t stop_at) {
  _lwid = in->get_id();
  return read_children_iff(in, stop_at);
}

////////////////////////////////////////////////////////////////////
//     Function: LwoHeader::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoHeader::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  indent(out, indent_level + 2)
    << _lwid << "\n";
  write_children(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}
