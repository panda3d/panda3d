// Filename: eggAnimPreload.cxx
// Created by:  drose (06Aug08)
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

#include "eggAnimPreload.h"

#include "string_utils.h"
#include "indent.h"

TypeHandle EggAnimPreload::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggAnimPreload::write
//       Access: Public, Virtual
//  Description: Writes the table and all of its children to the
//               indicated output stream in Egg format.
////////////////////////////////////////////////////////////////////
void EggAnimPreload::
write(ostream &out, int indent_level) const {
  test_under_integrity();

  write_header(out, indent_level, "<AnimPreload>");

  if (has_fps()) {
    indent(out, indent_level + 2)
      << "<Scalar> fps { " << get_fps() << " }\n";
  }

  if (has_num_frames()) {
    indent(out, indent_level + 2)
      << "<Scalar> frames { " << get_num_frames() << " }\n";
  }

  indent(out, indent_level) << "}\n";
}
