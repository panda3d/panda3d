// Filename: lwoHeader.cxx
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

#include "lwoHeader.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoHeader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoHeader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LwoHeader::
LwoHeader() {
  _valid = false;
  _version = 0.0;
}

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
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _lwid = lin->get_id();

  if (_lwid == IffId("LWO2")) {
    _valid = true;
    _version = 6.0;
  } else if (_lwid == IffId("LWOB")) {
    _valid = true;
    _version = 5.0;
  }

  if (_valid) {
    lin->set_lwo_version(_version);
  }

  read_chunks_iff(lin, stop_at);

  return true;
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
    << "id = " << _lwid << "\n";
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}
