// Filename: lwoBoundingBox.cxx
// Created by:  drose (24Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "lwoBoundingBox.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoBoundingBox::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoBoundingBox::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoBoundingBox::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _min = lin->get_vec3();
  _max = lin->get_vec3();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoBoundingBox::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LwoBoundingBox::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { min = " << _min << ", max = " << _max << " }\n";
}
