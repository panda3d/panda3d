// Filename: eggNamedObject.cxx
// Created by:  drose (16Jan99)
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
