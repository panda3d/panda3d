// Filename: lwoSurfaceBlockImage.cxx
// Created by:  drose (24Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "lwoSurfaceBlockImage.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoSurfaceBlockImage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockImage::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoSurfaceBlockImage::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _index = lin->get_vx();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoSurfaceBlockImage::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LwoSurfaceBlockImage::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { index = " << _index << " }\n";
}
