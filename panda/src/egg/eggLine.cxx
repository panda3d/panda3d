// Filename: eggLine.cxx
// Created by:  drose (14Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "eggLine.h"

#include "indent.h"

TypeHandle EggLine::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggLine::write
//       Access: Public, Virtual
//  Description: Writes the point to the indicated output stream in
//               Egg format.
////////////////////////////////////////////////////////////////////
void EggLine::
write(ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Line>");
  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}
