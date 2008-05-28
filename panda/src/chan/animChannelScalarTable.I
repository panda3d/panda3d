// Filename: animChannelScalarTable.I
// Created by:  drose (22Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::has_table
//       Access: Published
//  Description: Returns true if the data table has been assigned.
////////////////////////////////////////////////////////////////////
INLINE bool AnimChannelScalarTable::
has_table() const {
  return _table != (const float *)NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::clear_table
//       Access: Published
//  Description: Empties the data table.
////////////////////////////////////////////////////////////////////
INLINE void AnimChannelScalarTable::
clear_table() {
  _table = NULL;
}

