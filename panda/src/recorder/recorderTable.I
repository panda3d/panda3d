// Filename: recorderTable.I
// Created by:  drose (27Jan04)
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


////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderTable::
RecorderTable() {
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderTable::
RecorderTable(const RecorderTable &copy) :
  _recorders(copy._recorders),
  _error(copy._error)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RecorderTable::
operator = (const RecorderTable &copy) {
  _recorders = copy._recorders;
  _error = copy._error;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RecorderTable::
~RecorderTable() {
}
