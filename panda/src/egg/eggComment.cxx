// Filename: eggComment.cxx
// Created by:  drose (20Jan99)
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
