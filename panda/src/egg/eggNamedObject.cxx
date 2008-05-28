// Filename: eggNamedObject.cxx
// Created by:  drose (16Jan99)
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

#include "eggNamedObject.h"
#include "eggMiscFuncs.h"

#include "indent.h"

TypeHandle EggNamedObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggNamedObject::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggNamedObject::
output(ostream &out) const {
  out << get_type();
  if (has_name()) {
    out << " " << get_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggNamedObject::write_header
//       Access: Public
//  Description: Writes the first line of the egg object,
//               e.g. "<Group> group_name {" or some such.  It
//               automatically enquotes the name if it contains any
//               special characters.  egg_keyword is the keyword that
//               begins the line, e.g. "<Group>".
////////////////////////////////////////////////////////////////////
void EggNamedObject::
write_header(ostream &out, int indent_level, const char *egg_keyword) const {
  indent(out, indent_level) << egg_keyword << " ";

  if (has_name()) {
    enquote_string(out, get_name()) << " {\n";
  } else {
    out << "{\n";
  }
}
