// Filename: eggCharacterDb.I
// Created by:  drose (05Oct06)
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
//     Function: EggCharacterDb::Key::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggCharacterDb::Key::
Key(const EggJointPointer *joint, TableType table_type, int frame) :
  _joint(joint),
  _table_type(table_type),
  _frame(frame)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterDb::Key::operator <
//       Access: Public
//  Description: Provides an arbitrary unique ordering for all keys.
////////////////////////////////////////////////////////////////////
INLINE bool EggCharacterDb::Key::
operator < (const EggCharacterDb::Key &other) const {
  if (_joint != other._joint) {
    return _joint < other._joint;
  }
  if (_table_type != other._table_type) {
    return _table_type < other._table_type;
  }
  return _frame < other._frame;
}
