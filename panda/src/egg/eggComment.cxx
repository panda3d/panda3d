// Filename: eggComment.cxx
// Created by:  drose (20Jan99)
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

#include "eggComment.h"
#include "eggMiscFuncs.h"

#include "indent.h"
#include "string_utils.h"

TypeHandle EggComment::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggComment::write
//       Access: Public, Virtual
//  Description: Writes the comment definition to the indicated output
//               stream in Egg format.
////////////////////////////////////////////////////////////////////
void EggComment::
write(ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Comment>");
  enquote_string(out, get_comment(), indent_level + 2) << "\n";
  indent(out, indent_level) << "}\n";
}
