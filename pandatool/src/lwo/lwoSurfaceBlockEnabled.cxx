// Filename: lwoSurfaceBlockEnabled.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoSurfaceBlockEnabled.h"
#include "lwoInputFile.h"

#include <indent.h>

TypeHandle LwoSurfaceBlockEnabled::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockEnabled::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoSurfaceBlockEnabled::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _enabled = (lin->get_be_uint16() != 0);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockEnabled::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoSurfaceBlockEnabled::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { enabled = " << _enabled << " }\n";
}
