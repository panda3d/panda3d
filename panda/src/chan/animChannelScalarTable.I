// Filename: animChannelScalarTable.I
// Created by:  drose (22Feb99)
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
//     Function: AnimChannelScalarTable::has_table
//       Access: Public
//  Description: Returns true if the data table has been assigned.
////////////////////////////////////////////////////////////////////
INLINE bool AnimChannelScalarTable::
has_table() const {
  return _table != (const float *)NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::clear_table
//       Access: Public
//  Description: Empties the data table.
////////////////////////////////////////////////////////////////////
INLINE void AnimChannelScalarTable::
clear_table() {
  _table = NULL;
}

