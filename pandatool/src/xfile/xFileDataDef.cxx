// Filename: xFileDataDef.cxx
// Created by:  drose (03Oct04)
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

#include "xFileDataDef.h"
#include "indent.h"

TypeHandle XFileDataDef::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileDataDef::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
XFileDataDef::
~XFileDataDef() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataDef::clear
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void XFileDataDef::
clear() {
  XFileNode::clear();
  _array_def.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataDef::add_array_def
//       Access: Public
//  Description: Adds an additional array dimension to the data
//               description.
////////////////////////////////////////////////////////////////////
void XFileDataDef::
add_array_def(const XFileArrayDef &array_def) {
  _array_def.push_back(array_def);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataDef::write_text
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataDef::
write_text(ostream &out, int indent_level) const {
  indent(out, indent_level);

  if (!_array_def.empty()) {
    out << "array ";
  }

  switch (_type) {
  case T_word:
    out << "WORD";
    break;

  case T_dword:
    out << "DWORD";
    break;

  case T_float:
    out << "FLOAT";
    break;

  case T_double:
    out << "DOUBLE";
    break;

  case T_char:
    out << "CHAR";
    break;

  case T_uchar:
    out << "UCHAR";
    break;

  case T_sword:
    out << "SWORD";
    break;

  case T_sdword:
    out << "SDWORD";
    break;

  case T_string:
    out << "STRING";
    break;

  case T_cstring:
    out << "CSTRING";
    break;

  case T_unicode:
    out << "UNICODE";
    break;

  case T_template:
    out << _template->get_name();
    break;
  }

  if (has_name()) {
    out << " " << get_name();
  }

  ArrayDef::const_iterator ai;
  for (ai = _array_def.begin(); ai != _array_def.end(); ++ai) {
    (*ai).output(out);
  }

  out << ";\n";
}
