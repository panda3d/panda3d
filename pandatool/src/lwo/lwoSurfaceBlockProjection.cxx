// Filename: lwoSurfaceBlockProjection.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoSurfaceBlockProjection.h"
#include "lwoInputFile.h"

#include <indent.h>

TypeHandle LwoSurfaceBlockProjection::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockProjection::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoSurfaceBlockProjection::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _mode = (Mode)lin->get_be_uint16();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockProjection::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoSurfaceBlockProjection::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { mode = " << (int)_mode << " }\n";
}
