// Filename: eggTable.I
// Created by:  drose (19Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: EggTable::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTable::
EggTable(const string &name) : EggGroupNode(name) {
  _type = TT_table;
}


////////////////////////////////////////////////////////////////////
//     Function: EggTable::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTable::
EggTable(const EggTable &copy) : EggGroupNode(copy), _type(copy._type) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggTable::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTable &EggTable::
operator = (const EggTable &copy) {
  EggGroupNode::operator = (copy);
  _type = copy._type;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggTable::set_table_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggTable::
set_table_type(TableType type) {
  _type = type;
}


////////////////////////////////////////////////////////////////////
//     Function: EggTable::get_table_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTable::TableType EggTable::
get_table_type() const {
  return _type;
}
