// Filename: eggTable.cxx
// Created by:  drose (19Feb99)
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

#include "eggTable.h"

#include <string_utils.h>
#include <indent.h>

TypeHandle EggTable::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggTable::write
//       Access: Public, Virtual
//  Description: Writes the table and all of its children to the
//               indicated output stream in Egg format.
////////////////////////////////////////////////////////////////////
void EggTable::
write(ostream &out, int indent_level) const {
  test_under_integrity();

  switch (get_table_type()) {
  case TT_table:
    write_header(out, indent_level, "<Table>");
    break;

  case TT_bundle:
    write_header(out, indent_level, "<Bundle>");
    break;

  default:
    // invalid table type
    nassertv(false);
  }

  EggGroupNode::write(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}


////////////////////////////////////////////////////////////////////
//     Function: EggTable::string_table_type
//       Access: Public, Static
//  Description: Returns the TableType value associated with the given
//               string representation, or TT_invalid if the string
//               does not match any known TableType value.
////////////////////////////////////////////////////////////////////
EggTable::TableType EggTable::
string_table_type(const string &string) {
  if (cmp_nocase_uh(string, "table") == 0) {
    return TT_table;
  } else if (cmp_nocase_uh(string, "bundle") == 0) {
    return TT_bundle;
  } else {
    return TT_invalid;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TableType output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggTable::TableType t) {
  switch (t) {
  case EggTable::TT_invalid:
    return out << "invalid table";
  case EggTable::TT_table:
    return out << "table";
  case EggTable::TT_bundle:
    return out << "bundle";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}
