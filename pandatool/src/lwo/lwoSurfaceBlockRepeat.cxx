// Filename: lwoSurfaceBlockRepeat.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoSurfaceBlockRepeat.h"
#include "lwoInputFile.h"

#include <indent.h>

TypeHandle LwoSurfaceBlockRepeat::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockRepeat::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoSurfaceBlockRepeat::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _cycles = lin->get_be_float32();
  _envelope = lin->get_vx();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockRepeat::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoSurfaceBlockRepeat::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { cycles = " << _cycles
    << ", envelope = " << _envelope << " }\n";
}
