// Filename: lwoSurfaceBlockRefObj.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoSurfaceBlockRefObj.h"
#include "lwoInputFile.h"

#include <indent.h>

TypeHandle LwoSurfaceBlockRefObj::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockRefObj::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoSurfaceBlockRefObj::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _name = lin->get_string();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockRefObj::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoSurfaceBlockRefObj::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { name = \"" << _name << "\" }\n";
}
